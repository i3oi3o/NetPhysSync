// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ServerPawnPrediction.h"
#include "UNPSNetSetting.h"
#include "FAutonomousProxyInput.h"
#include "FNPS_StaticHelperFunction.h"
#include "NPSLogCategory.h"



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
	, SyncClientTickIndexWithProcessedServerTick(0)
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

	ensureMsgf(TestIndex <= 1, TEXT("Should call cosecutively by server tick index."));
#endif

	const TArray<FSavedInput>& ProxyInputBuffer = AutonomousProxyInput
		.GetArray();
	uint32 ProxyClientTickStartIndex = AutonomousProxyInput
		.GetArrayStartClientTickIndex();
	uint32 BeginCopyClientTickIndex = ProxyClientTickStartIndex;
	int32 BeginCopyProxyInputIndex = 0;

	// Ignore already processed input.
	if (bHasLastProcessedInputClientTickIndex)
	{
		int32 Index;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			ProxyClientTickStartIndex, 
			LastProcessedClientInputTickIndex+1,
			Index
		);
		if (Index >= ProxyInputBuffer.Num())
		{
			// This can happen when client continue to send unacknowledged input
			// and Server have processed all input. 
			// Server just need to send correction till client don't have any unacknowledged.
			UE_LOG
			(
				LogNPS_Net, Log, TEXT("Discard old input. ProxyStartTick:%u, ProxyNum:%d, SendClientTickStamp:%u"),
				ProxyClientTickStartIndex, ProxyInputBuffer.Num(),
				AutonomousProxyInput.SendTickStamp
			);
			return;
		}
		else if(Index >= 0)
		{
			BeginCopyProxyInputIndex = Index;
			BeginCopyClientTickIndex = ProxyClientTickStartIndex + Index;
		}
	}

	if (!IsProcessingClientInput())
	{
		bHasSyncClientTickIndex = true;
		
		int32 WaitJitter = NetSetting->JitterWaitPhysTick;

		if (WaitJitter > ProxyInputBuffer.Num())
		{
			InputStartServerTickIndex = ReceiveServerTickIndex + WaitJitter;
		}
		else
		{
			InputStartServerTickIndex = ReceiveServerTickIndex;
		}

		SyncClientTickIndexWithProcessedServerTick =
			BeginCopyClientTickIndex
			+ FNPS_StaticHelperFunction::CalculateBufferArrayIndex
			(
				InputStartServerTickIndex, LastProcessedServerTickIndex
			);
		

		InputStartClientTickIndex = BeginCopyClientTickIndex;

		InputBuffer.Empty();
		for (int32 i = BeginCopyProxyInputIndex; i < ProxyInputBuffer.Num(); ++i)
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
			BeginCopyClientTickIndex,
			OutArrayIndex
		);

		uint32 OldInputClientTickIndex = InputStartClientTickIndex;

		int32 CopyIndex;
		if (OutArrayIndex >= 0)
		{
			CopyIndex = BeginCopyProxyInputIndex;
		}
		else
		{
			// Discard index older than InputStartClientTickIndex.
			CopyIndex = BeginCopyProxyInputIndex - OutArrayIndex;
		}

		for (int32 i = CopyIndex; i < ProxyInputBuffer.Num(); ++i)
		{
			FNPS_StaticHelperFunction::SetElementToBuffers
			(
				InputBuffer, ProxyInputBuffer[i],
				InputStartClientTickIndex,
				ProxyClientTickStartIndex + i
			);
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

	if (InputBuffer.Num() > 0)
	{
		if (InputBuffer[InputBuffer.Num() - 1].IsEmptyInput())
		{
			UE_LOG
			(
				LogNPS_Net, Log,
				TEXT("Server receive end input at the end of buffer. AtServerTick:%u"),
				InputStartServerTickIndex + InputBuffer.Num() - 1
			);
		}

		UE_LOG
		(
			LogNPS_Net, Log,
			TEXT("Buffer ClientStartInput:%u, ServerStartInput:%u, NumInput:%d. ProxyBStartTick:%u, ProxyBNum:%d, ProxyBBeginCopyTick:%u, ProxySendTickStamp:%u"),
			InputStartClientTickIndex, InputStartServerTickIndex, InputBuffer.Num(),
			ProxyClientTickStartIndex, ProxyInputBuffer.Num(),
			BeginCopyProxyInputIndex,
			AutonomousProxyInput.SendTickStamp
		);
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

					if (InputBuffer.Num() == 0)
					{
						UE_LOG
						(
							LogNPS_Net, Log, TEXT("Processed All Input - Client Tick Index:%u, SavedInput:%s"),
							LastProcessedClientInputTickIndex,
							*ToReturn->ToString()
						);
					}
					else
					{
						UE_LOG
						(
							LogNPS_Net, Log, TEXT("Processed Input - Client Tick Index:%u, SavedInput:%s"),
							LastProcessedClientInputTickIndex,
							*ToReturn->ToString()
						);
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
					UE_LOG
					(
						LogNPS_Net, Log, 
						TEXT("Processed Input - Wait for more input. ServerTick:%u, WaitForClientTick:%u"),
						ServerTickIndex, InputStartClientTickIndex + InputBuffer.Num()
					);
					// Cancel SyncClientTickIndex advancement code below because
					// We cannot process missing input.
					SyncClientTickIndexWithProcessedServerTick -= AdvanceAmount;
					
					
				}
				else
				{
					ensureMsgf(false, TEXT("Shouldn't reach here."));
				}
			}
		}
		

		if (bHasSyncClientTickIndex)
		{
			SyncClientTickIndexWithProcessedServerTick += AdvanceAmount;
		}
		
		bHasLastProcessedInputClientTickIndex = bHasLastProcessedInputClientTickIndex &&
			!FNPS_StaticHelperFunction::IsCachTickTooOld
			(
				LastProcessedClientInputTickIndex,
				SyncClientTickIndexWithProcessedServerTick
			);
	}


	LastProcessedServerTickIndex = ServerTickIndex;
	bHasLastProcessedServerTickIndex = true;

#if !UE_BUILD_SHIPPING
	// Invoke method to check sync client tick.
	GetSyncClientTickIndex(ServerTickIndex);
#endif

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

uint32 FNPS_ServerPawnPrediction::GetSyncClientTickIndex(uint32 ServerTick) const
{
	int32 Diff;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		LastProcessedServerTickIndex, 
		ServerTick,
		Diff
	);

	uint32 ToReturn = SyncClientTickIndexWithProcessedServerTick + Diff;
#if !UE_BUILD_SHIPPING
	if (InputBuffer.Num() > 0)
	{
		int32 DebugDiff;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			InputStartServerTickIndex,
			ServerTick,
			DebugDiff
		);


		ensureMsgf
		(
			ToReturn == (InputStartClientTickIndex + DebugDiff), 
			TEXT("Wrong sync client tick calculation.")
		);
	}

#endif

	return ToReturn;
}
