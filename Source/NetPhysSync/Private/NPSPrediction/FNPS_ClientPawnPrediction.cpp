// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ClientPawnPrediction.h"

FNPS_ClientPawnPrediction::FNPS_ClientPawnPrediction()
	: Super()
	, ClientInputBuffers(20)
	, ClientInputBuffersStartTickIndex(0)
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
				ClientInputBuffers.Num() > 0 &&
				!ClientInputBuffers[ClientInputBuffers.Num()-1].IsEmptyInput()
			)
	   )
	{
		if (ClientInputBuffers.Num() == 0)
		{
			OldestUnacknowledgedInput = ClientTickIndex;
		}

		FNPS_StaticHelperFunction::SetElementToBuffers
		(
			ClientInputBuffers, ToSave,
			ClientInputBuffersStartTickIndex, ClientTickIndex
		);

	}
}

const FSavedInput& FNPS_ClientPawnPrediction::GetSavedInput(uint32 ClientTick, bool bUseNearestIfOutOfBound /*=true*/) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(
		ClientInputBuffersStartTickIndex, ClientTick, OutArrayIndex);

	if (bUseNearestIfOutOfBound)
	{
		ClientInputBuffers.ClampIndexParamWithinRange(OutArrayIndex);
	}

	if (ClientInputBuffers.IsIndexInRange(OutArrayIndex))
	{
		return ClientInputBuffers[OutArrayIndex];
	}
	else
	{
		return FSavedInput::EmptyInput;
	}
}


bool FNPS_ClientPawnPrediction::HasUnacknowledgedInput() const
{
	return ClientInputBuffers.Num() > 0;
}

uint32 FNPS_ClientPawnPrediction::GetOldestUnacknowledgeInputClientTickIndex() const
{
	return OldestUnacknowledgedInput;
}

void FNPS_ClientPawnPrediction::ShiftElementsToDifferentTickIndex(int32 ShiftAmount)
{
	Super::ShiftElementsToDifferentTickIndex(ShiftAmount);
	ClientInputBuffersStartTickIndex += ShiftAmount;
	OldestUnacknowledgedInput += ShiftAmount;
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
			ClientInputBuffersStartTickIndex,
			ClientTickIndex,
			OutArrayIndex
		);

		if (OutArrayIndex >= ClientInputBuffers.Num())
		{
			OldestUnacknowledgedInput = ClientInputBuffersStartTickIndex
				+ ClientInputBuffers.Num() - 1;
			ClientInputBuffers.Empty();
		}
		else
		{
			OldestUnacknowledgedInput = ClientTickIndex;
		}
	}
}

