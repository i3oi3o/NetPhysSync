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
	, LastProcessedClientInputTickIndex(0)
	, bHasLastProcessedInputClientTickIndex(false)
	, SyncClientTickIndexForStampRigidBody(0)
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


	if (!IsProcessingClientInput())
	{
		bHasSyncClientTickIndex = true;
		
		int32 WaitJitter = NetSetting->JitterWaitPhysTick;

		if (WaitJitter > ProxyInputBuffer.Num())
		{
			SyncClientTickIndexForStampRigidBody = ProxyClientTickStartIndex - WaitJitter;
			InputStartServerTickIndex = ReceiveServerTickIndex + WaitJitter;
		}
		else
		{
			SyncClientTickIndexForStampRigidBody = ProxyClientTickStartIndex;
			InputStartServerTickIndex = ReceiveServerTickIndex;
		}

		InputStartClientTickIndex = ProxyClientTickStartIndex;

		InputBuffer.Empty();
		for (int32 i = 0; i < ProxyInputBuffer.Num(); ++i)
		{
			if (InputBuffer.IsFull())
			{
				++InputStartClientTickIndex;
				++InputStartServerTickIndex;
			}
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

		uint32 OldInputClientTickIndex = InputStartClientTickIndex;

		if (OutArrayIndex >= 0)
		{
			for (int32 i = 0; i < ProxyInputBuffer.Num(); ++i)
			{
				FNPS_StaticHelperFunction::SetElementToBuffers
				(
					InputBuffer, ProxyInputBuffer[i], 
					InputStartClientTickIndex,
					ProxyClientTickStartIndex+i
				);
			}
		}
		else
		{
			for (int32 i = -OutArrayIndex; i < ProxyInputBuffer.Num(); ++i)
			{
				FNPS_StaticHelperFunction::SetElementToBuffers
				(
					InputBuffer, ProxyInputBuffer[i],
					InputStartClientTickIndex,
					OldInputClientTickIndex + i + OutArrayIndex
				);
			}
		}

		int32 ShiftAmount;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			OldInputClientTickIndex,
			InputStartClientTickIndex,
			ShiftAmount
		);

		InputStartServerTickIndex += ShiftAmount;
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

		if (IsProcessingClientInput())
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
					LastProcessedClientInputTickIndex = InputStartClientTickIndex + ToProcessedIndex;
					if ( 
							InputBuffer.IsLastIndex(ToProcessedIndex) &&
							InputBuffer[ToProcessedIndex].IsEmptyInput() // No more input.
					   )
					{
						InputBuffer.Empty();
					}
					else
					{
						ToReturn = &(InputBuffer[ToProcessedIndex]);
					}
				}
				// Buffer is not end yet. 
				// Because of drop package and latency, We don't receive continuing buffer yet.
				else if(!(InputBuffer[InputBuffer.Num()-1].IsEmptyInput()))
				{
					/**
					 * NOTE :This implementation can cause too long buffer on client.
					 * - Consider downstream implementation later as recommended by 
					 * Rocket League physic synchronization.
				     * - Another option is upstream as recommended by Overwatch, 
					 * But this technique require more effort in implementation.
					 */

					// Shift buffer element to different server tick index.
					int32 ShiftAmount = ToProcessedIndex - InputBuffer.Num() + 1;
					InputStartServerTickIndex += ShiftAmount;
					ensureMsgf(bHasSyncClientTickIndex, TEXT("Should have sync client tick index."));
					// Cancel SyncClientTickIndex advancement code below because
					// We cannot processed missing input.
					SyncClientTickIndexForStampRigidBody -= AdvanceAmount;
				}
				else
				{
					ensureMsgf(false, TEXT("Shouldn't reach here."));
				}
			}
		}
		

		if (bHasSyncClientTickIndex)
		{
			SyncClientTickIndexForStampRigidBody += AdvanceAmount;
		}
		
		bHasLastProcessedInputClientTickIndex = bHasLastProcessedServerTickIndex &&
			!FNPS_StaticHelperFunction::IsCachTickTooOld
			(
				LastProcessedClientInputTickIndex,
				SyncClientTickIndexForStampRigidBody
			);
	}


	LastProcessedServerTickIndex = ServerTickIndex;
	bHasLastProcessedServerTickIndex = true;

	return *ToReturn;
}

bool FNPS_ServerPawnPrediction::HasUnprocessedInputForSimulatedProxy() const
{
	uint32 CurrentUnprocessedServerTick = LastProcessedServerTickIndex + 1;
	int32 UnprocessedArrayIndex = 0;

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		CurrentUnprocessedServerTick,
		InputStartServerTickIndex,
		UnprocessedArrayIndex
	);

	return IsProcessingClientInput() &&
		(
			UnprocessedArrayIndex >= 0 ||
			(
				-UnprocessedArrayIndex >= 0 &&
				-UnprocessedArrayIndex < InputBuffer.Num()
			)
		);
}
