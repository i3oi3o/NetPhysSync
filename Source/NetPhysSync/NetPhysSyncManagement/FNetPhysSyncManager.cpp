// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNetPhysSyncManager.h"
#include <PhysicsPublic.h>
#include "INetPhysSync.h"

FNetPhysSyncManager::FNetPhysSyncManager()
	: PhysScene(nullptr)
	, WorldOwningPhysScene(nullptr)
	, LocalNetPhysTicks(0)
	, INetPhysSyncPtrList()
	, StartTickPostPhysicSubstep(false)
	, ForAssertDataRace(false)
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
	checkf(!ForAssertDataRace, TEXT("No Adding or Removing during Start, Substep, PostSubstep, End."));
	INetPhysSyncPtrList.Add(iNetPhysSyncPtr);
}

void FNetPhysSyncManager::UnregisterINetPhysSync(INetPhysSyncPtr iNetPhysSyncPtr)
{
	checkf(!ForAssertDataRace, TEXT("No Adding or Removing during Start, Substep, PostSubstep, End."))
	INetPhysSyncPtrList.Remove(iNetPhysSyncPtr);
}

void FNetPhysSyncManager::OnTickPostPhysic()
{
	FPostPhysStepParam PostStepParam(PhysScene, CachSceneType, CachStepDeltaTime, LocalNetPhysTicks);
	for (auto It = INetPhysSyncPtrList.CreateIterator(); It; ++It)
	{
		INetPhysSync* Interface = TryGetTickableINetPhysSync(*It);
		if (Interface != nullptr)
		{
			Interface->TickPostPhysStep(PostStepParam);
		}
	}

	FEndPhysParam EndParam(CachSceneType, CachStartDeltaTime, LocalNetPhysTicks);
	for (auto It = INetPhysSyncPtrList.CreateIterator(); It; ++It)
	{
		INetPhysSync* Interface = TryGetTickableINetPhysSync(*It);
		if (Interface != nullptr)
		{
			Interface->TickEndPhysic(EndParam);
		}
	}

	ForAssertDataRace = false;

}

void FNetPhysSyncManager::TickStartPhys(FPhysScene* PhysScene, uint32 SceneType, float StartDeltaTime)
{
	ForAssertDataRace = true;
	StartTickPostPhysicSubstep = false;
	CachStartDeltaTime = StartDeltaTime;
	CachSceneType = SceneType;

	FStartPhysParam StartParam(PhysScene, SceneType, StartDeltaTime);
	for (auto It = INetPhysSyncPtrList.CreateIterator(); It; ++It)
	{
		INetPhysSync* Interface = TryGetTickableINetPhysSync(*It);
		if (Interface != nullptr)
		{
			Interface->TickStartPhysic(StartParam);
		}
	}
}

void FNetPhysSyncManager::TickStepPhys(FPhysScene* PhysScene, uint32 SceneType, float StepDeltaTime)
{
	if (StartTickPostPhysicSubstep)
	{
		FPostPhysStepParam PostStepParam(PhysScene, SceneType, StepDeltaTime, LocalNetPhysTicks);
		for (auto It = INetPhysSyncPtrList.CreateIterator(); It; ++It)
		{
			INetPhysSync* Interface = TryGetTickableINetPhysSync(*It);
			if (Interface != nullptr)
			{
				Interface->TickPostPhysStep(PostStepParam);
			}
		}
	}
	else
	{
		CachStepDeltaTime = StepDeltaTime;
	}

	++LocalNetPhysTicks;

	FPhysStepParam StepParam(PhysScene, SceneType, StepDeltaTime);

	for (auto It = INetPhysSyncPtrList.CreateIterator(); It; ++It)
	{
		INetPhysSync* Interface = TryGetTickableINetPhysSync(*It);
		if (Interface != nullptr)
		{
			Interface->TickPhysStep(StepParam);
		}
	}

	StartTickPostPhysicSubstep = true;
}

INetPhysSync* FNetPhysSyncManager::TryGetTickableINetPhysSync(const INetPhysSyncPtr& TargetPtr)
{
	INetPhysSync* ToReturn = (INetPhysSync*)TargetPtr.GetInterface();

	if (ToReturn != nullptr && ToReturn->IsTickEnabled())
	{
		return ToReturn;
	}
	else
	{
		return nullptr;
	}
	
}
