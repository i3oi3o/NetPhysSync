// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ClientPawnPrediction.h"
#include "NPSLogCategory.h"

FNPS_ClientPawnPrediction::FNPS_ClientPawnPrediction()
	: Super()
	, ClientInputBuffer(20)
	, ClientInputBufferStartTickIndex(0)
	, bIsOldestUnacknowledgeInputTooOld(true)
{
}

FNPS_ClientPawnPrediction::~FNPS_ClientPawnPrediction()
{
}

void FNPS_ClientPawnPrediction::SaveInput(FVector TargetWorldSpeed, uint32 ClientTickIndex)
{
	SaveInput(FSavedInput(TargetWorldSpeed), ClientTickIndex);
}

void FNPS_ClientPawnPrediction::SaveInput(const FSavedInput& ToSave, uint32 ClientTickIndex)
{

	if (
			!ToSave.IsEmptyInput() ||
			(
				// This condition prevent putting many empty inputs 
				// at the end of buffer.
				ClientInputBuffer.Num() > 0 &&
				!ClientInputBuffer[ClientInputBuffer.Num()-1].IsEmptyInput()
			)
	   )
	{
		if (ClientInputBuffer.Num() == 0)
		{
			bIsOldestUnacknowledgeInputTooOld = false;
			OldestUnacknowledgedInputTick = ClientTickIndex;
		}

		FNPS_StaticHelperFunction::SetElementToBuffers
		(
			ClientInputBuffer, ToSave,
			ClientInputBufferStartTickIndex, ClientTickIndex
		);

	}
}

const FSavedInput& FNPS_ClientPawnPrediction::GetSavedInput
(
	uint32 ClientTick, 
	EIdxOutOfRangeHandle eHandleOutOfBoundIndex /*=EIdxOutOfRangeHandle::UseNearestIndex*/
) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(
		ClientInputBufferStartTickIndex, ClientTick, OutArrayIndex);


	HandleOutOfBoundIndex
	(
		ClientInputBuffer,
		eHandleOutOfBoundIndex,
		OutArrayIndex
	);	

	if (ClientInputBuffer.IsIndexInRange(OutArrayIndex))
	{
		return ClientInputBuffer[OutArrayIndex];
	}
	else
	{
		return FSavedInput::EmptyInput;
	}
}

bool FNPS_ClientPawnPrediction::TryGetOldestUnacknowledgeInputTickIndex(uint32& OutTickIndex) const
{
	OutTickIndex = OldestUnacknowledgedInputTick;
	return !bIsOldestUnacknowledgeInputTooOld;
}

void FNPS_ClientPawnPrediction::ShiftElementsToDifferentTickIndex(int32 ShiftAmount)
{
	Super::ShiftElementsToDifferentTickIndex(ShiftAmount);
	ClientInputBufferStartTickIndex += ShiftAmount;
	OldestUnacknowledgedInputTick += ShiftAmount;
}

void FNPS_ClientPawnPrediction::ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex)
{
	Super::ServerCorrectState(CorrectState, ClientTickIndex);


	if (
			// This expression mean accepting correct tick index by based class.
			// Base class ignores correct state with too old tick.
			LastCorrectedStateTickIndex == ClientTickIndex 
			&& 
			HasUnacknowledgedInput()
	   )
	{
		int32 OutArrayIndex;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			ClientInputBufferStartTickIndex,
			ClientTickIndex,
			OutArrayIndex
		);

		if (OutArrayIndex >= ClientInputBuffer.Num())
		{
			OldestUnacknowledgedInputTick = ClientInputBufferStartTickIndex
				+ ClientInputBuffer.Num() - 1;
			ClientInputBuffer.Empty();
		}
		else
		{
			OldestUnacknowledgedInputTick = ClientTickIndex;
		}

#if NPS_LOG_SYNC_AUTO_PROXY
		UE_LOG(LogNPS_Net, Log, TEXT("UnacknowledgeInput after Acknowledging:%d"), OldestUnacknowledgedInputTick);
#endif
		
		bIsOldestUnacknowledgeInputTooOld = false;
	}
#if NPS_LOG_SYNC_AUTO_PROXY
	else
	{
		if (LastCorrectedStateTickIndex != ClientTickIndex)
		{
			UE_LOG(LogNPS_Net, Log, TEXT("Correct state is ignored. So, no acknowledge input."));
		}
		else if (!HasUnacknowledgedInput())
		{
			UE_LOG(LogNPS_Net, Log, TEXT("No input to acknowledge."));
		}
	}
#endif
}

void FNPS_ClientPawnPrediction::Update(uint32 CurrentTickIndex)
{
	Super::Update(CurrentTickIndex);
	if (!bIsOldestUnacknowledgeInputTooOld)
	{
		bIsOldestUnacknowledgeInputTooOld = FNPS_StaticHelperFunction::
			IsCachTickTooOld(OldestUnacknowledgedInputTick, CurrentTickIndex);
	}
}


