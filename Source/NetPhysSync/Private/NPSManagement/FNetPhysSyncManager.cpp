// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNetPhysSyncManager.h"
#include <PhysicsPublic.h>
#include "INetPhysSync.h"
#include "INetPhysSyncParam.h"
#include "FNPS_StaticHelperFunction.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysXPublic.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "UNPSNetSetting.h"
#include "IOnReplayEnd.h"
#include "NPSLogCategory.h"

using namespace physx;

#if WITH_PHYSX
#if WITH_APEX
typedef apex::Scene PxApexScene;	//helper typedef so we don't have to use as many ifdefs
#else
typedef PxScene PxApexScene;
#endif
#endif

FNetPhysSyncManager::FNetPhysSyncManager(AActor* OwningActorParam)
	: PhysScene(nullptr)
	, WorldOwningPhysScene(nullptr)
	, LocalPhysTickIndex(0)
	, INetPhysSyncPtrList()
	, bStartTickPostPhysicSubstepYet(false)
	, bStartPhysicYet(false)
	, PxScratchReplayBuffer(nullptr)
	, PxScratchReplayBufferSize(0)
	, OwningActor(OwningActorParam)
{
	checkf(OwningActor != nullptr, TEXT("Missing owning actor."));
	int32 SceneScratchBufferSize = UPhysicsSettings::Get()->SimulateScratchMemorySize;
	checkf(SceneScratchBufferSize > 0, TEXT("SceneScratchBufferSize should be more than zero."));

	int32 SimScratchBufferBoundary = 16 * 1024; // 16 KB as required by PhysX.
	SceneScratchBufferSize = FMath::DivideAndRoundUp<int32>
		(SceneScratchBufferSize, SimScratchBufferBoundary) 
		* SimScratchBufferBoundary;

	// Need to allocate alignment memory.
	PxScratchReplayBuffer = static_cast<uint8*>
		(FMemory::Malloc(SceneScratchBufferSize, 16));
	PxScratchReplayBufferSize = SceneScratchBufferSize;
}

FNetPhysSyncManager::~FNetPhysSyncManager()
{
	if (WorldOwningPhysScene.IsValid() && !WorldOwningPhysScene->IsPendingKill())
	{
		this->PhysScene->OnPhysScenePreTick.Remove(TickStartPhysHandle);
		this->PhysScene->OnPhysSceneStep.Remove(TickStepPhysHandle);
	}
	WorldOwningPhysScene = nullptr;
	PhysScene = nullptr;

	checkf(PxScratchReplayBuffer != nullptr, TEXT("Why is PxScratchReplayBuffer null?"));
	FMemory::Free(PxScratchReplayBuffer);
	PxScratchReplayBuffer = nullptr;
	PxScratchReplayBufferSize = 0;
	OwningActor = nullptr;
}

void FNetPhysSyncManager::Initialize(FPhysScene* PhysScene)
{
	WorldOwningPhysScene = PhysScene->GetOwningWorld();
	checkf(WorldOwningPhysScene.Get() != nullptr, TEXT("Cannot accept FPhysScene not own by UWorld."));
	this->PhysScene = PhysScene;
	TickStartPhysHandle = this->PhysScene->OnPhysScenePreTick.AddRaw(this, &FNetPhysSyncManager::TickStartPhys);
	TickStepPhysHandle = this->PhysScene->OnPhysSceneStep.AddRaw(this, &FNetPhysSyncManager::TickStepPhys);
}

void FNetPhysSyncManager::RegisterINetPhysSync(INetPhysSyncPtr iNetPhysSyncPtr)
{
	if (!DeferedRegister.Contains(iNetPhysSyncPtr))
	{
		DeferedRegister.Add(iNetPhysSyncPtr);
	}

	int ToRemoveIndex;
	if (DeferedUnregister.Find(iNetPhysSyncPtr, ToRemoveIndex))
	{
		DeferedUnregister.RemoveAt(ToRemoveIndex, 1, false);
	}
}

void FNetPhysSyncManager::UnregisterINetPhysSync(INetPhysSyncPtr iNetPhysSyncPtr)
{
	int ToRemoveIndex;
	if (DeferedRegister.Find(iNetPhysSyncPtr, ToRemoveIndex))
	{
		DeferedRegister.RemoveAt(iNetPhysSyncPtr, 1, false);
	}
	
	if (!DeferedRegister.Contains(iNetPhysSyncPtr))
	{
		DeferedUnregister.Add(iNetPhysSyncPtr);
	}
}

void FNetPhysSyncManager::OnTickPrePhysic()
{
	if (!DoWeNeedReplay() || WorldOwningPhysScene == nullptr)
	{
		return;
	}

	// Currently only support PST_Sync;
	EPhysicsSceneType SceneTypeEnum = EPhysicsSceneType::PST_Sync;
	FIsTickEnableParam IsTickEnableParam(SceneTypeEnum);

#if !UE_BUILD_SHIPPING
	uint32 DebugNewestReceivedServerTick;
	bool bHasDebugNewstReceivedServerTick;
	bHasDebugNewstReceivedServerTick = 
		TryGetNewestUnprocessedServerTick(IsTickEnableParam, DebugNewestReceivedServerTick);
#endif

	FTickSyncPoint OldSyncPoint = CurrentSyncPoint;
	FTickSyncPoint QuerySyncPoint;
	if (TryGetNewSyncPoint(IsTickEnableParam, QuerySyncPoint))
	{
		CurrentSyncPoint = QuerySyncPoint;
	}

	if(!CurrentSyncPoint.IsValid())
	{
		return;
	}

#if !UE_BUILD_SHIPPING
	static bool LogOnce = true;
	if (LogOnce)
	{
		LogOnce = false;
		UE_LOG(LogNPS_Net, Log, TEXT("SyncPoint is available. Ready to simulate."));
	}
#endif

	FOnNewSyncPointInfo OnNewSyncPointInfo
	(
		OldSyncPoint,
		CurrentSyncPoint
	);



	FOnReadReplicationParam OnReadReplicationParam
	(
		OnNewSyncPointInfo
	);
	CallINetPhysSyncFunction
	(
		&INetPhysSync::OnReadReplication,
		OnReadReplicationParam,
		IsTickEnableParam
	);

	uint32 OutReplayIndex;
	if (TryGetReplayIndex(IsTickEnableParam, OnNewSyncPointInfo, OutReplayIndex))
	{
		int32 ReplayNumber;

		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			OutReplayIndex,
			LocalPhysTickIndex,
			ReplayNumber
		);

		if (ReplayNumber < 0)
		{
			/*
				Handle this later. Possible cause :
					- Client run slowly than server.
					- This is a bug.
			*/
#if !UE_BUILD_SHIPPING
			if (bHasDebugNewstReceivedServerTick)
			{
				UE_LOG
				(
					LogNPS_Net, Log,
					TEXT("We get negative replay number. ReplayTick: %u, CurrentTick: %u, ReceivedServerTick: %u, CurrentSyncPoint: %s"),
					OutReplayIndex,
					LocalPhysTickIndex,
					DebugNewestReceivedServerTick,
					*CurrentSyncPoint.ToString()
				);
			}
#endif
			CallINetPhysSyncFunction
			(
				&INetPhysSync::OnFinishUsingReplication,
				FOnFinishUsingReplicationParam(),
				IsTickEnableParam
			);

			return;
		}


		LocalPhysTickIndex = OutReplayIndex;
		FPhysScene* PhysScene = WorldOwningPhysScene->GetPhysicsScene();
		FReplayStartParam ReplayStartParam
		(
			PhysScene,
			SceneTypeEnum,
			OnNewSyncPointInfo,
			LocalPhysTickIndex
		);
		
		CallINetPhysSyncFunction
		(
			&INetPhysSync::TickReplayStart,
			ReplayStartParam,
			IsTickEnableParam
		);

		float SubstepDeltaTime = UPhysicsSettings::Get()->MaxSubstepDeltaTime;
		PxApexScene* PhysXScene = nullptr;

#if WITH_APEX
		PhysXScene = PhysScene->GetApexScene(SceneTypeEnum);
#else
		PhysXScene = PhysScene->GetPhysXScene(SceneTypeEnum);
#endif

		for (int32 i = 0; i < ReplayNumber; ++i)
		{
			/**
			 * Cannot use PhysScene::TickPhysic(Arg...) or PhysScene::StartFrame(Arg...) here.
			 * In doing so, can cause deadlock when waiting for task to complete.
			 */
			FReplaySubstepParam ReplaySubstepParam
			(
				PhysScene,
				SceneTypeEnum,
				SubstepDeltaTime,
				LocalPhysTickIndex
			);
			CallINetPhysSyncFunction
			(
				&INetPhysSync::TickReplaySubstep,
				ReplaySubstepParam,
				IsTickEnableParam
			);

			uint32 OutErrorCode = 0;
#if WITH_APEX
			PhysXScene->simulate
			(
				SubstepDeltaTime, i == ReplayNumber-1,
				nullptr, PxScratchReplayBuffer, 
				PxScratchReplayBufferSize
			);
			PhysXScene->fetchResults(true, &OutErrorCode);
#else
			PhysXScene->lockWrite();
			PhysXScene->simulate
			(
				SubstepDeltaTime, nullptr, PxScratchReplayBuffer,
				PxScratchReplayBufferSize
			);
			
			PhysXScene->fetchResults(true, &OutErrorCode);
			PhysXScene->unlockWrite();
#endif
			if (OutErrorCode != 0)
			{
				/**
				 * Create log category for this case later.
				 */
				UE_LOG(LogTemp, Log, TEXT("REPLAY PHYSX FETCHRESULTS ERROR: %d"), OutErrorCode);
			}

			/**
			 * FPhysScene hold event notification from PhysX.
			 * My modified engine source code always dispatch event before calling post step
			 */
			PhysScene->DispatchPhysNotifications_AssumesLocked();

			FReplayPostStepParam ReplayPostStepParam
			(
				PhysScene,
				SceneTypeEnum,
				SubstepDeltaTime,
				// This get increased in delegate callback.
				LocalPhysTickIndex
			);
			CallINetPhysSyncFunction
			(
				&INetPhysSync::TickReplayPostSubstep,
				ReplayPostStepParam,
				IsTickEnableParam
			);

			++LocalPhysTickIndex;
		}

		checkf
		(
			LocalPhysTickIndex == OutReplayIndex + ReplayNumber, 
			TEXT("Replay steps are incomplete.")
		);

		FReplayEndParam ReplayEndParam(SceneTypeEnum, LocalPhysTickIndex);
		CallINetPhysSyncFunction
		(
			&INetPhysSync::TickReplayEnd,
			ReplayEndParam,
			IsTickEnableParam
		);

		checkf(OwningActor != nullptr, TEXT("Why is owning actor null?"));
		IOnReplayEnd* ReplayEndInterface = Cast<IOnReplayEnd>(OwningActor);
		if (ReplayEndInterface != nullptr)
		{
			ReplayEndInterface->OnReplayEnd();
		}
	}

	CallINetPhysSyncFunction
	(
		&INetPhysSync::OnFinishUsingReplication,
		FOnFinishUsingReplicationParam(),
		IsTickEnableParam
	);
}

void FNetPhysSyncManager::OnTickPostPhysic(float GameFrameDeltaTime)
{
	if (bStartPhysicYet)
	{
		bStartPhysicYet = false;
		FIsTickEnableParam IsTickEnableParam(CachSceneType);
		FPostPhysStepParam PostStepParam(PhysScene, CachSceneType, CachStepDeltaTime, LocalPhysTickIndex);
		CallINetPhysSyncFunction
		(
			&INetPhysSync::TickPostPhysStep,
			PostStepParam, IsTickEnableParam
		);
		++LocalPhysTickIndex;

		FEndPhysParam EndParam(CachSceneType, CachStartDeltaTime, LocalPhysTickIndex);
		CallINetPhysSyncFunction
		(
			&INetPhysSync::TickEndPhysic,
			EndParam, IsTickEnableParam
		);
	}

	if (DoWeNeedReplay() && CurrentSyncPoint.IsValid())
	{
		FVisualUpdateParam VisualUpdateParam(GameFrameDeltaTime);
		CallINetPhysSyncFunction
		(
			&INetPhysSync::VisualUpdate,
			VisualUpdateParam, FIsTickEnableParam(EPhysicsSceneType::PST_Sync)
		);
	}
}

bool FNetPhysSyncManager::DoWeNeedReplay() const
{
	return OwningActor != nullptr &&
		OwningActor->IsNetMode(NM_Client);
}

void FNetPhysSyncManager::TickStartPhys(FPhysScene* PhysScene, uint32 SceneType, float StartDeltaTime)
{
	if (
		 SceneType != EPhysicsSceneType::PST_Sync || 
		 (DoWeNeedReplay() && !CurrentSyncPoint.IsValid()) // Wait until we have sync point.
	   )
	{
		// Currently support only PST_Sync
		return;
	}

	bStartPhysicYet = true;
	bStartTickPostPhysicSubstepYet = false;
	CachStartDeltaTime = StartDeltaTime;
	CachSceneType = static_cast<EPhysicsSceneType>(SceneType);

	FlushDeferedRegisteeAndCleanNull();

	FIsTickEnableParam IsTickEnableParam(CachSceneType);
	FStartPhysParam StartParam(PhysScene, SceneType, StartDeltaTime, LocalPhysTickIndex);
	CallINetPhysSyncFunction
	(
		&INetPhysSync::TickStartPhysic,
		StartParam, IsTickEnableParam
	);
}

void FNetPhysSyncManager::TickStepPhys(FPhysScene* PhysScene, uint32 SceneType, float StepDeltaTime)
{
	if (!bStartPhysicYet)
	{
		return;
	}

	EPhysicsSceneType SceneTypeEnum = static_cast<EPhysicsSceneType>(SceneType);
	FIsTickEnableParam IsTickEnableParam(SceneTypeEnum);

	if (bStartTickPostPhysicSubstepYet)
	{
		FPostPhysStepParam PostStepParam(PhysScene, SceneTypeEnum, StepDeltaTime, LocalPhysTickIndex);
		CallINetPhysSyncFunction
		(
			&INetPhysSync::TickPostPhysStep, 
			PostStepParam, IsTickEnableParam
		);
		++LocalPhysTickIndex;
	}
	else
	{
		CachStepDeltaTime = StepDeltaTime;
	}

	FPhysStepParam StepParam(PhysScene, SceneTypeEnum, StepDeltaTime, LocalPhysTickIndex);

	CallINetPhysSyncFunction
	(
		&INetPhysSync::TickPhysStep,
		StepParam, IsTickEnableParam
	);

	bStartTickPostPhysicSubstepYet = true;
	
}

void FNetPhysSyncManager::FlushDeferedRegisteeAndCleanNull()
{
	INetPhysSyncPtrList.Append(DeferedRegister);

	DeferedRegister.Empty();

	for (auto It = DeferedUnregister.CreateIterator(); It; ++It)
	{
		int ToRemoveIndex;
		if (INetPhysSyncPtrList.Find(*It, ToRemoveIndex))
		{
			INetPhysSyncPtrList.RemoveAt(ToRemoveIndex, 1, false);
		}
	}

	DeferedUnregister.Empty();

	int idx = 0;
	while (idx < INetPhysSyncPtrList.Num())
	{
		if (INetPhysSyncPtrList[idx] == nullptr)
		{
			INetPhysSyncPtrList.RemoveAt(idx, 1, false);
		}
		else
		{
			++idx;
		}
	}
}

bool FNetPhysSyncManager::TryGetNewestUnprocessedServerTick
(
	const FIsTickEnableParam& IsTickEnableParam,
	uint32& OutNewestUnprocessedServerTick
)
{
	bool bFoundYet = false;

	const IQueryReceivedPackage* QueryInterface = Cast<IQueryReceivedPackage>(OwningActor);
	uint32 Query = 0;
	bFoundYet = 
	(
		QueryInterface != nullptr &&
		QueryInterface->TryGetNewestUnprocessedServerTick(Query)
	);

	for (auto It = INetPhysSyncPtrList.CreateIterator(); It; ++It)
	{
		INetPhysSyncPtr InterfacePtr = *It;
		uint32 NewQueryServerTick;
		if (
				InterfacePtr != nullptr &&
				InterfacePtr->IsTickEnabled(IsTickEnableParam) &&
				InterfacePtr->TryGetNewestUnprocessedServerTick(NewQueryServerTick)
		   )
		{
			if (!bFoundYet)
			{
				bFoundYet = true;
				Query = NewQueryServerTick;
			}
			else
			{
				
				int32 Diff;
				FNPS_StaticHelperFunction::CalculateBufferArrayIndex
				(
					Query,
					NewQueryServerTick,
					Diff
				);

				if (Diff > 0)
				{
					Query = NewQueryServerTick;
				}
			}
		}
	}

	if (bFoundYet)
	{
		OutNewestUnprocessedServerTick = Query;
	}

	return bFoundYet;
}

bool FNetPhysSyncManager::TryGetNewSyncPoint
(
	const FIsTickEnableParam& IsTickEnableParam,
	FTickSyncPoint& OutSyncPoint
)
{
	if (WorldOwningPhysScene != nullptr && DoWeNeedReplay())
	{
		for (int32 i = 0; i < INetPhysSyncPtrList.Num(); ++i)
		{
			if (INetPhysSyncPtrList[i] != nullptr &&
				INetPhysSyncPtrList[i]->IsTickEnabled(IsTickEnableParam) &&
				INetPhysSyncPtrList[i]->IsLocalPlayerControlPawn())
			{
				if (INetPhysSyncPtrList[i]->TryGetNewSyncTick(OutSyncPoint))
				{
					return true;
				}
				else
				{
					break;
				}
			}
		}

		// Wait for ping calculation a bit.
		if (WorldOwningPhysScene->GetRealTimeSeconds() < 3)
		{
			return false;
		}



		uint32 NewestUnprocessedServerTick;
		if (TryGetNewestUnprocessedServerTick(IsTickEnableParam, NewestUnprocessedServerTick))
		{
			int32 DiffExistServerTick = TNumericLimits<int32>::Max();

			if (CurrentSyncPoint.IsValid())
			{
				DiffExistServerTick = FNPS_StaticHelperFunction::CalculateBufferArrayIndex
				(
					CurrentSyncPoint.GetServerTickSyncPoint(),
					NewestUnprocessedServerTick
				);
			}

			// Use threshold value from configure file later.
			bool IsCurrentSyncPointTooOld = DiffExistServerTick < -NPS_BUFFER_SIZE || 
				DiffExistServerTick > NPS_BUFFER_SIZE;

			if (!IsCurrentSyncPointTooOld)
			{
				return false;
			}

			const APlayerController* PlayerController = WorldOwningPhysScene->GetFirstPlayerController();
			
			if (PlayerController != nullptr && 
				PlayerController->PlayerState != nullptr)
			{
				float RTT = PlayerController->PlayerState->ExactPing;

				if (RTT > 0)
				{
					RTT *= 0.001f;
					UNPSNetSetting* NetSetting = FNPS_StaticHelperFunction
						::GetNetSetting();
					float StepDeltaTime = UPhysicsSettings::Get()->
						MaxSubstepDeltaTime;
					int32 TotalPredictSteps = FMath::FloorToInt(RTT / StepDeltaTime) 
						+ NetSetting->JitterWaitPhysTick;

					// Current client tick will sync with predict NewServerSyncTick.
					uint32 NewClientSyncTick = LocalPhysTickIndex;
					uint32 NewServerSyncTick = NewestUnprocessedServerTick + 
						TotalPredictSteps;
					OutSyncPoint = FTickSyncPoint(NewClientSyncTick, 
						NewServerSyncTick);

					checkf
					(
						OutSyncPoint.GetClientTickSyncPoint() == NewClientSyncTick &&
						OutSyncPoint.GetServerTickSyncPoint() == NewServerSyncTick,
						TEXT("Typo creating OutSyncPoint.")
					);
					return true;
				}
				
			}
		}
	}
	
	return false;
	
}

bool FNetPhysSyncManager::TryGetReplayIndex
(
	const FIsTickEnableParam& IsTickEnableParam,
	const FOnNewSyncPointInfo& OnNewSyncPointInfo ,
	uint32& OutReplayIndex
)
{
	OutReplayIndex = 0;
	if (WorldOwningPhysScene != nullptr)
	{
		int32 OldestDiff = TNumericLimits<int32>::Max();

		// Need replay to fill missing history buffer.
		if (OnNewSyncPointInfo.ShiftClientTickAmountForReplayPrediction < 0)
		{
			OldestDiff = OnNewSyncPointInfo.ShiftClientTickAmountForReplayPrediction;
			OutReplayIndex = LocalPhysTickIndex + OldestDiff;

#if !UE_BUILD_SHIPPING
			int32 OutDebugDiff;
			FNPS_StaticHelperFunction::CalculateBufferArrayIndex
			(
				LocalPhysTickIndex,
				OutReplayIndex,
				OutDebugDiff
			);

			ensureMsgf(OldestDiff == OutDebugDiff, TEXT("Wrong diff calculation for replaying to fill missing history buffer."));
#endif
		}

		for (int32 i = 0; i < INetPhysSyncPtrList.Num(); ++i)
		{
			uint32 QueryReplayTickIndex;
			if (INetPhysSyncPtrList[i] != nullptr &&
				INetPhysSyncPtrList[i]->IsTickEnabled(IsTickEnableParam) &&
				INetPhysSyncPtrList[i]->TryGetReplayIndex(QueryReplayTickIndex))
			{
				int32 NewDiff;
				FNPS_StaticHelperFunction::CalculateBufferArrayIndex
				(
					LocalPhysTickIndex,
					QueryReplayTickIndex,
					NewDiff
				);

				// Find the oldest.
				if (NewDiff < OldestDiff)
				{
					OldestDiff = NewDiff;
					OutReplayIndex = QueryReplayTickIndex;
				}
			}
		}

		return OldestDiff != TNumericLimits<int32>::Max();
	}

	return false;
}
