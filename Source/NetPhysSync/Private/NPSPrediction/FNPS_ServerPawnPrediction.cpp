// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ServerPawnPrediction.h"
#include "UNPSNetSetting.h"
#include "FAutonomousProxyInput.h"
#include "FNPS_StaticHelperFunction.h"



FNPS_ServerPawnPrediction::FNPS_ServerPawnPrediction
(
	const UNPSNetSetting* const NetSettingParam
)
	: Super()
	, InputBuffer(NPS_BUFFER_SIZE)
	, NetSetting(NetSettingParam)
	, InputStartServerTickIndex(0)
	, InputStartClientTickIndex(0)
	, LastProcessedClientTickIndex(0)
	, bHasLastProcessedInputClientTickIndex(false)
	, SyncClientTickIndex(0)
	, bHasSyncClientTickIndex(false)
	, LastProcessedServerTickIndex(0)
	, bHasLastProcessedServerTickIndex(false)
{

}

FNPS_ServerPawnPrediction::~FNPS_ServerPawnPrediction()
{
}

void FNPS_ServerPawnPrediction::UpdateInputBuffer
	(
		const FAutonomousProxyInput& AutonomousProxyInput, 
		uint32 ReceiveServerTickIndex
	)
{
	if (!bHasLastProcessedServerTickIndex)
	{
		bHasLastProcessedServerTickIndex = true;
		LastProcessedServerTickIndex = ReceiveServerTickIndex - 1;
	}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	int32 TestIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		LastProcessedServerTickIndex,
		ReceiveServerTickIndex,
		TestIndex
	);

	ensureMsgf(TestIndex == 1, TEXT("Should call cosecutively by server tick index."));
#endif


	const TArray<FSavedInput>& ProxyInputBuffer = AutonomousProxyInput
		.GetArray();
	uint32 ProxyClientTickStartIndex = AutonomousProxyInput
		.GetArrayStartClientTickIndex();


	if (!HasUnprocessedInput())
	{
		bHasSyncClientTickIndex = true;
		
		int32 WaitJitter = NetSetting->JitterWaitPhysTick;

		if (WaitJitter < ProxyInputBuffer.Num())
		{
			SyncClientTickIndex = ProxyClientTickStartIndex - WaitJitter;
			InputStartServerTickIndex = ReceiveServerTickIndex + WaitJitter;
		}
		else
		{
			SyncClientTickIndex = ProxyClientTickStartIndex;
			InputStartServerTickIndex = ReceiveServerTickIndex;
		}

		InputStartClientTickIndex = ProxyClientTickStartIndex;

		InputBuffer.Empty();
		for (int32 i = 0; i < ProxyInputBuffer.Num(); ++i)
		{
			InputBuffer.Add(ProxyInputBuffer[i]);
		}
	}
	else
	{
		int32 OutArrayIndex;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			InputStartClientTickIndex,
			ProxyClientTickStartIndex,
			OutArrayIndex
		);

		int32 LastIndex = ProxyInputBuffer.Num() - 1 + OutArrayIndex;
		int32 CapacityIndex = InputBuffer.Capacity() - 1;
		if (LastIndex > CapacityIndex)
		{
			int32 Overflow = LastIndex - CapacityIndex;
			InputStartClientTickIndex += Overflow;
			InputStartServerTickIndex += Overflow;
		}

		for (int32 i = 0; i < ProxyInputBuffer.Num(); ++i)
		{
			int32 BufferIndex = OutArrayIndex + i;

			if (BufferIndex >= 0)
			{
				if (BufferIndex >= InputBuffer.Num())
				{
					InputBuffer.Add(ProxyInputBuffer[i]);
				}
				else
				{
					InputBuffer[i] = ProxyInputBuffer[BufferIndex];
				}
			}
		}
	}


}

const FSavedInput& FNPS_ServerPawnPrediction::ProcessServerTick(uint32 ServerTickIndex)
{
	const FSavedInput* ToReturn = &(FSavedInput::EmptyInput);

	if (bHasLastProcessedServerTickIndex)
	{
		int32 AdvanceAmount;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			LastProcessedServerTickIndex,
			ServerTickIndex,
			AdvanceAmount
		);

		ensureMsgf(AdvanceAmount == 1, TEXT("Should call cosecutively by server tick index."));

		if (HasUnprocessedInput())
		{
			int32 ToProcessedIndex;

			FNPS_StaticHelperFunction::CalculateBufferArrayIndex
			(
				InputStartServerTickIndex,
				ServerTickIndex,
				ToProcessedIndex
			);

			if (ToProcessedIndex >= 0)
			{
				bHasLastProcessedInputClientTickIndex = true;
				

				
				if (InputBuffer.IsIndexInRange(ToProcessedIndex))
				{
					LastProcessedClientTickIndex = InputStartClientTickIndex + ToProcessedIndex;
					if ( 
							InputBuffer.IsLastIndex(ToProcessedIndex) &&
							InputBuffer[ToProcessedIndex].IsEmptyInput()
					   )
					{
						InputBuffer.Empty();
					}
					else
					{
						ToReturn = &(InputBuffer[ToProcessedIndex]);
					}
				}
				else if(!(InputBuffer[InputBuffer.Num()-1].IsEmptyInput()))
				{
					// Shift buffer element to different server tick index.
					InputStartServerTickIndex += ToProcessedIndex - InputBuffer.Num() + 1;
				}
			}
		}

		if (bHasSyncClientTickIndex)
		{
			SyncClientTickIndex += AdvanceAmount;
		}
	}


	LastProcessedServerTickIndex = ServerTickIndex;
	bHasLastProcessedServerTickIndex = true;

	return *ToReturn;
}
