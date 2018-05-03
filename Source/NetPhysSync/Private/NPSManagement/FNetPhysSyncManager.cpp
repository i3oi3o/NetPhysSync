// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNetPhysSyncManager.h"
#include <PhysicsPublic.h>
#include "INetPhysSync.h"
#include "FNPS_StaticHelperFunction.h"
#include "PhysicsEngine/PhysicsSettings.h"

FNetPhysSyncManager::FNetPhysSyncManager()
	: PhysScene(nullptr)
	, WorldOwningPhysScene(nullptr)
	, LocalPhysTickIndex(0)
	, INetPhysSyncPtrList()
	, bStartTickPostPhysicSubstepYet(false)
	, bStartPhysicYet(false)
{
}

FNetPhysSyncManager::~FNetPhysSyncManager()
{
	if (WorldOwningPhysScene.IsValid())
	{
		this->PhysScene->OnPhysScenePreTick.Remove(TickStartPhysHandle);
		this->PhysScene->OnPhysSceneStep.Remove(TickStepPhysHandle);
	}
	WorldOwningPhysScene = nullptr;
	PhysScene = nullptr;
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
	if (TryGetReplayIndex(OutReplayIndex) && WorldOwningPhysScene != nullptr)
	{
		bIsReplaying = true;
		int32 ReplayNumber;

		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			OutReplayIndex,
			LocalPhysTickIndex,
			ReplayNumber
		);

		checkf(ReplayNumber > 0, TEXT("Why isn't replay number positive?"));
		LocalPhysTickIndex = OutReplayIndex;

		bStartTickReplayStepYet = false;
		// Currently only support PST_Sync;
		EPhysicsSceneType SceneTypeEnum = EPhysicsSceneType::PST_Sync;
		FIsTickEnableParam IsTickEnableParam(SceneTypeEnum);
		
		FPhysScene* PhysScene = WorldOwningPhysScene->GetPhysicsScene();
		FReplayStartParam ReplayStartParam
		(
			PhysScene,
			SceneTypeEnum,
			FOnNewSyncPointInfo(FTickSyncPoint(0, 0), FTickSyncPoint(0, 0)),
			LocalPhysTickIndex
		);
		
		CallINetPhysSyncFunction
		(
			&INetPhysSync::TickReplayStart,
			ReplayStartParam,
			IsTickEnableParam
		);

		FVector DefaultGravity(0.0f, 0.0f, WorldOwningPhysScene->GetGravityZ());
		float SubstepDeltaTime = UPhysicsSettings::Get()->MaxSubstepDeltaTime;
		float MaxPhysicsDeltaTime = UPhysicsSettings::Get()->MaxPhysicsDeltaTime;
		for (int32 i = 0; i < ReplayNumber; ++i)
		{
			PhysScene->SetUpForFrame
			(
				&DefaultGravity,
				SubstepDeltaTime,
				MaxPhysicsDeltaTime
			);
			PhysScene->StartFrame();
			PhysScene->WaitPhysScenes();
			PhysScene->EndFrame(nullptr);
		}

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
	if (!bStartPhysicYet || bIsReplaying)
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
	if (SceneType != EPhysicsSceneType::PST_Sync || bIsReplaying)
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
	if (bIsReplaying)
	{
		/**
		 * Route delegate call to replay.
		 */
		TickStepRelay(PhysScene, SceneType, StepDeltaTime);
		return;
	}

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

void FNetPhysSyncManager::TickStepRelay(FPhysScene* PhysScene, uint32 SceneType, float StepDeltaTime)
{
	EPhysicsSceneType SceneTypeEnum = static_cast<EPhysicsSceneType>(SceneType);
	FIsTickEnableParam IsTickEnableParam(SceneTypeEnum);

	if (bStartTickReplayStepYet)
	{
		FReplayPostStepParam ReplayPostStepParam
		(
			PhysScene, SceneTypeEnum, StepDeltaTime, LocalPhysTickIndex
		);

		CallINetPhysSyncFunction
		(
			&INetPhysSync::TickReplayPostSubstep,
			ReplayPostStepParam, IsTickEnableParam
		);
		++LocalPhysTickIndex;
	}

	FReplaySubstepParam ReplayStepParam
	(
		PhysScene, SceneTypeEnum, StepDeltaTime, LocalPhysTickIndex
	);

	CallINetPhysSyncFunction
	(
		&INetPhysSync::TickReplaySubstep,
		ReplayStepParam, IsTickEnableParam
	);

	bStartTickReplayStepYet = true;
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
