// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNetPhysSyncManager.h"
#include <PhysicsPublic.h>
#include "INetPhysSync.h"

FNetPhysSyncManager::FNetPhysSyncManager()
	: PhysScene(nullptr)
	, WorldOwningPhysScene(nullptr)
	, LocalPhysTickIndex(0)
	, INetPhysSyncPtrList()
	, StartTickPostPhysicSubstepYet(false)
	, StartPhysicYet(false)
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

void FNetPhysSyncManager::OnTickPostPhysic()
{
	if (!StartPhysicYet)
	{
		return;
	}

	StartPhysicYet = false;
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
	checkf(!StartPhysicYet, TEXT("Currently only support sync physic scene."));
	StartPhysicYet = true;
	StartTickPostPhysicSubstepYet = false;
	CachStartDeltaTime = StartDeltaTime;
	CachSceneType = SceneType;

	FlushDeferedRegisteeAndCleanNull();

	FIsTickEnableParam IsTickEnableParam(SceneType);
	FStartPhysParam StartParam(PhysScene, SceneType, StartDeltaTime, LocalPhysTickIndex);
	CallINetPhysSyncFunction
	(
		&INetPhysSync::TickStartPhysic,
		StartParam, IsTickEnableParam
	);
}

void FNetPhysSyncManager::TickStepPhys(FPhysScene* PhysScene, uint32 SceneType, float StepDeltaTime)
{
	FIsTickEnableParam IsTickEnableParam(SceneType);

	if (StartTickPostPhysicSubstepYet)
	{
		FPostPhysStepParam PostStepParam(PhysScene, SceneType, StepDeltaTime, LocalPhysTickIndex);
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

	FPhysStepParam StepParam(PhysScene, SceneType, StepDeltaTime, LocalPhysTickIndex);

	CallINetPhysSyncFunction
	(
		&INetPhysSync::TickPhysStep,
		StepParam, IsTickEnableParam
	);

	StartTickPostPhysicSubstepYet = true;
	
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
