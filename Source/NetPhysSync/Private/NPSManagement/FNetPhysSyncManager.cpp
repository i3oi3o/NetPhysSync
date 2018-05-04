// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNetPhysSyncManager.h"
#include <PhysicsPublic.h>
#include "INetPhysSync.h"
#include "FNPS_StaticHelperFunction.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysXPublic.h"

using namespace physx;

#if WITH_PHYSX
#if WITH_APEX
typedef apex::Scene PxApexScene;	//helper typedef so we don't have to use as many ifdefs
#else
typedef PxScene PxApexScene;
#endif
#endif

FNetPhysSyncManager::FNetPhysSyncManager()
	: PhysScene(nullptr)
	, WorldOwningPhysScene(nullptr)
	, LocalPhysTickIndex(0)
	, INetPhysSyncPtrList()
	, bStartTickPostPhysicSubstepYet(false)
	, bStartPhysicYet(false)
	, PxScratchReplayBuffer(nullptr)
	, PxScratchReplayBufferSize(0)
{
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
	uint32 OutReplayIndex;

	FTickSyncPoint NewSyncPoint;
	bool bHasNewSyncPoint = false;
	if (TryGetNewSyncPoint(NewSyncPoint))
	{
		bHasNewSyncPoint = true;
	}

	FOnNewSyncPointInfo NewSyncPointInfo
	(
		CurrentSyncPoint,
		bHasNewSyncPoint ? NewSyncPoint : CurrentSyncPoint
	);

	// Currently only support PST_Sync;
	EPhysicsSceneType SceneTypeEnum = EPhysicsSceneType::PST_Sync;
	FIsTickEnableParam IsTickEnableParam(SceneTypeEnum);

	FOnReadReplicationParam OnReadReplicationParam
	(
		NewSyncPointInfo
	);
	CallINetPhysSyncFunction
	(
		&INetPhysSync::OnReadReplication,
		OnReadReplicationParam,
		IsTickEnableParam
	);


	if (TryGetReplayIndex(OutReplayIndex) && WorldOwningPhysScene != nullptr)
	{
		int32 ReplayNumber;

		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			OutReplayIndex,
			LocalPhysTickIndex,
			ReplayNumber
		);

		checkf(ReplayNumber > 0, TEXT("Why isn't replay number positive?"));
		LocalPhysTickIndex = OutReplayIndex;
		FPhysScene* PhysScene = WorldOwningPhysScene->GetPhysicsScene();
		FReplayStartParam ReplayStartParam
		(
			PhysScene,
			SceneTypeEnum,
			NewSyncPointInfo,
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
	}
}

void FNetPhysSyncManager::OnTickPostPhysic(float GameFrameDeltaTime)
{
	if (!bStartPhysicYet)
	{
		return;
	}

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

void FNetPhysSyncManager::TickStartPhys(FPhysScene* PhysScene, uint32 SceneType, float StartDeltaTime)
{
	if (SceneType != EPhysicsSceneType::PST_Sync)
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

bool FNetPhysSyncManager::TryGetNewSyncPoint(FTickSyncPoint& OutSyncPoint)
{
	if (WorldOwningPhysScene != nullptr)
	{
		for (int32 i = 0; i < INetPhysSyncPtrList.Num(); ++i)
		{
			if (INetPhysSyncPtrList[i]->IsLocalPlayerControlPawn())
			{
				return INetPhysSyncPtrList[i]->TryGetNewSyncTick(OutSyncPoint);
			}
		}
	}
	
	
	return false;
	
}

bool FNetPhysSyncManager::TryGetReplayIndex(uint32& OutReplayIndex)
{
	if (WorldOwningPhysScene != nullptr)
	{
		int32 OldestDiff = TNumericLimits<int32>::Max();

		for (int32 i = 0; i < INetPhysSyncPtrList.Num(); ++i)
		{
			uint32 QueryReplayTickIndex;
			if (INetPhysSyncPtrList[i]->TryGetReplayIndex(QueryReplayTickIndex))
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
