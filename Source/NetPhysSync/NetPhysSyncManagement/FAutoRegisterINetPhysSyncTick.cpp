// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FAutoRegisterINetPhysSyncTick.h"
#include "INetPhysSync.h"
#include <GameFramework/Actor.h>
#include <Engine/World.h>
#include <Components/ActorComponent.h>
#include "NPSGameState.h"


FAutoRegisterINetPhysSyncTick::FAutoRegisterINetPhysSyncTick()
	: Super(),
	ToRegister()
{
	this->TickGroup = ETickingGroup::TG_PrePhysics;
	this->bCanEverTick = true;
	this->bTickEvenWhenPaused = true;
	this->bAllowTickOnDedicatedServer = true;
	this->bRunOnAnyThread = false;
}

FAutoRegisterINetPhysSyncTick::~FAutoRegisterINetPhysSyncTick()
{
	ToRegister = nullptr;
}

void FAutoRegisterINetPhysSyncTick::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (ToRegister != nullptr)
	{
		UObject* InterfaceOwner = ToRegister.GetObject();
		UWorld* World = InterfaceOwner->GetWorld();

		if (World != nullptr)
		{
			ANPSGameState* GameState = World->GetGameState<ANPSGameState>();

			if (GameState != nullptr)
			{
				GameState->RegisterINetPhysSync(ToRegister);
				StopAutoRegister();
			}
		}
		else
		{
			StopAutoRegister();
		}
	}
	else
	{
		StopAutoRegister();
	}
}

FString FAutoRegisterINetPhysSyncTick::DiagnosticMessage()
{
	UObject* InterfaceOwner = ToRegister.GetObject();
	if (InterfaceOwner != nullptr)
	{
		return InterfaceOwner->GetFullName() + TEXT("[TickComponent]");
	}
	else
	{
		return TEXT("ToRegister Interface Become Null.");
	}
}

void FAutoRegisterINetPhysSyncTick::StartAutoRegister(TScriptInterface<INetPhysSync> ToRegisterParam)
{
	if (IsTickFunctionRegistered())
	{
		checkf(false, TEXT("This function is already registered."));
		return;
	}

	UObject* InterfaceOwner = ToRegisterParam.GetObject();
	ULevel* Level = nullptr;

	AActor* Actor = Cast<AActor>(InterfaceOwner);
	UActorComponent* ActorCompoenent = Cast<UActorComponent>(InterfaceOwner);

	checkf(Actor != nullptr || ActorCompoenent != nullptr, TEXT("FAutoRegisterINetPhysSyncTick only support AActor or UActorComponent."));

	if (ActorCompoenent != nullptr)
	{
		AActor* CompOwner = ActorCompoenent->GetOwner();
		if (CompOwner == nullptr)
		{
			Level = CompOwner->GetLevel();
		}
		else if(ActorCompoenent->GetWorld() != nullptr)
		{
			Level = ActorCompoenent->GetWorld()->PersistentLevel;
		}
	}

	if (Actor != nullptr)
	{
		checkf(Actor->GetLevel() != nullptr, TEXT("Actor is not in World yet. Please call this in BeginPlay"));
		Level = Actor->GetLevel();
	}
	
	if (Level != nullptr && ToRegisterParam != nullptr)
	{
		this->ToRegister = ToRegisterParam;
		RegisterTickFunction(Level);
	}
	else
	{
		checkf(false, TEXT("Cannot find level. Try call from BeginPlay."));
	}
}

void FAutoRegisterINetPhysSyncTick::StopAutoRegister()
{
	if (IsTickFunctionRegistered())
	{
		UnRegisterTickFunction();
		ToRegister = nullptr;
	}
}
