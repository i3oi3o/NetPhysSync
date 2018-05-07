// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_StaticHelperFunction.h"
#include "ScriptInterface.h"
#include "INetPhysSync.h"
#include "ANPSGameState.h"
#include "Engine/World.h"
#include "UNPSNetSetting.h"


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

void FNPS_StaticHelperFunction::UnregisterINetPhySync(TScriptInterface<INetPhysSync> ToUnregister)
{
	UObject* Obj = ToUnregister.GetObject();
	if (Obj != nullptr)
	{
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


