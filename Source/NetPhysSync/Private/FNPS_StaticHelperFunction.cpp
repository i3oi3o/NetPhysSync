// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_StaticHelperFunction.h"
#include "ScriptInterface.h"
#include "INetPhysSync.h"
#include "ANPSGameState.h"
#include "Engine/World.h"
#include "UNPSNetSetting.h"
#include "FAutoRegisterINetPhysSyncTick.h"

TArray<FAutoRegisterINetPhysSyncTick*> FNPS_StaticHelperFunction::AutoRegister_Running;
TArray<FAutoRegisterINetPhysSyncTick*> FNPS_StaticHelperFunction::AutoRegister_Pool;

void FNPS_StaticHelperFunction::CalculateBufferArrayIndex(uint32 BufferStartTickIndex, uint32 BufferTargetIndex, int32& OutResultArrayIndex)
{
	uint32 FirstResult = BufferTargetIndex - BufferStartTickIndex;
	uint32 SecondResult = BufferStartTickIndex - BufferTargetIndex;

	if (FirstResult < SecondResult)
	{
		OutResultArrayIndex = static_cast<int32>(FirstResult);
	}
	else
	{
		OutResultArrayIndex = -static_cast<int32>(SecondResult);
	}
}

int32 FNPS_StaticHelperFunction::CalculateBufferArrayIndex(uint32 BufferStartTickIndex, uint32 BufferTargetIndex)
{
	int32 ToReturn;
	CalculateBufferArrayIndex(BufferStartTickIndex, BufferTargetIndex, ToReturn);
	return ToReturn;
}

bool FNPS_StaticHelperFunction::IsCachTickTooOld(uint32 CachTick, uint32 CurrentTick)
{
	int32 Diff;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		CachTick,
		CurrentTick,
		Diff
	);

	int32 Threshold = FNPS_StaticHelperFunction::
		GetPositiveInclusiveThresholdForOldTick();

	return FMath::Abs(Diff) > Threshold;
}

int32 FNPS_StaticHelperFunction::GetPositiveInclusiveThresholdForOldTick()
{
	return TNumericLimits<int32>::Max() - NPS_BUFFER_SIZE;
}

void FNPS_StaticHelperFunction::RegisterINetPhySync(TScriptInterface<INetPhysSync> ToRegister)
{
	UObject* Obj = ToRegister.GetObject();
	if (Obj != nullptr && !Obj->IsPendingKill())
	{
		for (int32 i = 0; i < AutoRegister_Running.Num(); ++i)
		{
			if (AutoRegister_Running[i]->GetRegistee() == ToRegister)
			{
				return;
			}
		}

		UWorld* World = Obj->GetWorld();

		if (World != nullptr)
		{
			AGameStateBase* GameStateBase = World->GetGameState();
			ANPSGameState* NPSGameState = Cast<ANPSGameState>(GameStateBase);
			if (NPSGameState != nullptr)
			{
				NPSGameState->RegisterINetPhysSync(ToRegister);
			}
			else if (GameStateBase == nullptr)
			{
				// Waiting for GameState replication.
				// Do we need this?
				FAutoRegisterINetPhysSyncTick* AutoRegister = GetOrCreateAutoRegisterFromPool();
				AutoRegister->StartAutoRegister(ToRegister);
			}
		}
	}
}

void FNPS_StaticHelperFunction::UnregisterINetPhySync(TScriptInterface<INetPhysSync> ToUnregister)
{
	UObject* Obj = ToUnregister.GetObject();
	if (Obj != nullptr)
	{
		for (int32 i = 0; i < AutoRegister_Running.Num(); ++i)
		{
			if (AutoRegister_Running[i]->GetRegistee() == ToUnregister)
			{
				AutoRegister_Running[i]->StopAutoRegister();
				break;
			}
		}

		UWorld* World = Obj->GetWorld();

		if (World != nullptr)
		{
			ANPSGameState* GameState = World->GetGameState<ANPSGameState>();
			if (GameState != nullptr && !GameState->IsPendingKill())
			{
				GameState->UnregisterINetPhysSync(ToUnregister);
			}
		}
	}
}

uint32 FNPS_StaticHelperFunction::GetCurrentPhysTickIndex(const UObject* Obj)
{
	UWorld* World = Obj->GetWorld();

	checkf
	(
		World != nullptr && !World->IsPendingKill(),
		TEXT("World is not available.")
	);

	ANPSGameState* GameState = World->GetGameState<ANPSGameState>();
	checkf
	(
		GameState != nullptr && !GameState->IsPendingKill(),
		TEXT("GameState is not available.")
	);

	return GameState->GetCurrentPhysTickIndex();
}

UNPSNetSetting* FNPS_StaticHelperFunction::GetNetSetting()
{
	return UNPSNetSetting::Get();
}

void FNPS_StaticHelperFunction::ReturnAutoRegisterToPool(FAutoRegisterINetPhysSyncTick* ToReturn)
{
	for (int32 i = 0; i < AutoRegister_Running.Num(); ++i)
	{
		if (AutoRegister_Running[i] == ToReturn)
		{
			AutoRegister_Pool.Add(ToReturn);
			AutoRegister_Running.RemoveAt(i, 1, false);
			break;
		}
	}
}

FAutoRegisterINetPhysSyncTick* FNPS_StaticHelperFunction::GetOrCreateAutoRegisterFromPool()
{
	if (AutoRegister_Pool.Num() == 0)
	{
		AutoRegister_Pool.Add(new FAutoRegisterINetPhysSyncTick());
	}

	FAutoRegisterINetPhysSyncTick* ToReturn = AutoRegister_Pool[0];
	AutoRegister_Pool.RemoveAtSwap(0, 1, false);
	AutoRegister_Running.Add(ToReturn);
	return ToReturn;
}
