// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "NPSGameState.h"
#include "NetPhysSyncManagement/FNetPhysSyncManager.h"
#include "NetPhysSyncManagement/INetPhysSync.h"
#include <Engine/World.h>



ANPSGameState::ANPSGameState()
{
	bBeginDestroy = false;
}

void ANPSGameState::RegisterINetPhysSync(TScriptInterface<INetPhysSync> ToRegister)
{
	if (!bBeginDestroy)
	{
		GetOrCreateNetPhysSyncManager()->RegisterINetPhysSync(ToRegister);
	}
}

void ANPSGameState::UnregisterINetPhysSync(TScriptInterface<INetPhysSync> ToUnregister)
{
	if (!bBeginDestroy)
	{
		GetOrCreateNetPhysSyncManager()->UnregisterINetPhysSync(ToUnregister);
	}
}

void ANPSGameState::RegisterActorTickFunctions(bool bRegister)
{
	Super::RegisterActorTickFunctions(bRegister);
	if (bRegister)
	{
		PostPhysicTickFunction.Target = GetOrCreateNetPhysSyncManager();
		PostPhysicTickFunction.SetTickFunctionEnable(true);
		PostPhysicTickFunction.RegisterTickFunction(GetLevel());
	}
	else
	{
		if (PostPhysicTickFunction.IsTickFunctionRegistered())
		{
			PostPhysicTickFunction.UnRegisterTickFunction();
		}
	}
}

void ANPSGameState::BeginDestroy()
{
	bBeginDestroy = true;
	Super::BeginDestroy();
	if (NetPhysSyncManager != nullptr)
	{
		delete NetPhysSyncManager;
		NetPhysSyncManager = nullptr;
	}
}

class FNetPhysSyncManager* ANPSGameState::GetOrCreateNetPhysSyncManager()
{
	if (NetPhysSyncManager == nullptr)
	{
		NetPhysSyncManager = new FNetPhysSyncManager();
	}

	return NetPhysSyncManager;
}

void ANPSGameState::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = this->GetWorld();
	if (World != nullptr)
	{
		GetOrCreateNetPhysSyncManager()->Initialize(World->GetPhysicsScene());
	}
	else
	{
		checkf(false, TEXT("Missing World. Why?"))
	}
	
}

void FNPSGameStatePostPhysicsTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (Target != nullptr)
	{
		Target->OnTickPostPhysic();
	}
}