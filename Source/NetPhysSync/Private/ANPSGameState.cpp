// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "ANPSGameState.h"
#include "FNetPhysSyncManager.h"
#include "INetPhysSync.h"
#include <Engine/World.h>
#include "UNPSNetSetting.h"
#include "UnrealNetwork.h"


ANPSGameState::ANPSGameState()
{
	bBeginDestroy = false;
	bNewUnprocessedServerTick = false;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PrePhysics;
	PrimaryActorTick.bCanEverTick = true;

	RepServerTick = 0;
}

void ANPSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ANPSGameState, RepServerTick, COND_SimulatedOnly);
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

void ANPSGameState::OnTickPostPhysic(float DeltaTime)
{
	if (!bBeginDestroy)
	{
		FNetPhysSyncManager* NPSManager = GetOrCreateNetPhysSyncManager();
		NPSManager->OnTickPostPhysic(DeltaTime);
		RepServerTick = NPSManager->GetTickIndex();
	}
}

void ANPSGameState::RegisterActorTickFunctions(bool bRegister)
{
	Super::RegisterActorTickFunctions(bRegister);
	if (bRegister)
	{
		PostPhysicTickFunction.Target = this;
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

bool ANPSGameState::TryGetNewestUnprocessedServerTick(uint32& OutServerTickIndex) const
{
	// Implement this later.
	if (bNewUnprocessedServerTick)
	{
		OutServerTickIndex = RepServerTick;
	}

	return bNewUnprocessedServerTick;
}

void ANPSGameState::OnReplayEnd()
{
	// Server tick is processed in replay.
	bNewUnprocessedServerTick = false;
}

uint32 ANPSGameState::GetCurrentPhysTickIndex()
{
	checkf(!bBeginDestroy, TEXT("ANPSGameState is being destroyed."));
	return GetOrCreateNetPhysSyncManager()->GetTickIndex();
}

void ANPSGameState::OnRep_ServerTick()
{
	bNewUnprocessedServerTick = true;
}

class FNetPhysSyncManager* ANPSGameState::GetOrCreateNetPhysSyncManager()
{
	checkf(!bBeginDestroy, TEXT("Don't call this if we begin destroy."));
	if (NetPhysSyncManager == nullptr)
	{
		NetPhysSyncManager = new FNetPhysSyncManager(this);
	}

	return NetPhysSyncManager;
}

void ANPSGameState::BeginPlay()
{
	Super::BeginPlay();

	UNPSNetSetting* NetSetting = UNPSNetSetting::Get();

	UE_LOG
	(
		LogTemp, Log, TEXT("NetSetting: { SupportRTTInMS:%f, JitterWaitPhysTick:%d, BufferSize:%d }"),
		NetSetting->SupportRTTInMS,
		NetSetting->JitterWaitPhysTick,
		NPS_BUFFER_SIZE
	);

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

void ANPSGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bBeginDestroy)
	{
		GetOrCreateNetPhysSyncManager()->OnTickPrePhysic();
	}
}

void FNPSGameStatePostPhysicsTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (Target != nullptr)
	{
		Target->OnTickPostPhysic(DeltaTime);


	}
}
