// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ClientPawnPrediction.h"

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

const FSavedInput& FNPS_ClientPawnPrediction::GetSavedInput(uint32 ClientTick, bool bUseNearestIfOutOfBound /*=true*/) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(
		ClientInputBufferStartTickIndex, ClientTick, OutArrayIndex);

	if (bUseNearestIfOutOfBound)
	{
		ClientInputBuffer.ClampIndexParamWithinRange(OutArrayIndex);
	}

	if (ClientInputBuffer.IsIndexInRange(OutArrayIndex))
	{
		return ClientInputBuffer[OutArrayIndex];
	}
	else
	{
		return FSavedInput::EmptyInput;
	}
}


bool FNPS_ClientPawnPrediction::HasUnacknowledgedInput() const
{
	return ClientInputBuffer.Num() > 0;
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

		bIsOldestUnacknowledgeInputTooOld = false;
	}
}

FBufferInfo FNPS_ClientPawnPrediction::GetInputBufferInfo() const
{
	return FBufferInfo(ClientInputBufferStartTickIndex, ClientInputBuffer.Num());
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


