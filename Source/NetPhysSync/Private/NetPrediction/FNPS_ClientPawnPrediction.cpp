// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ClientPawnPrediction.h"
#include "FNPS_StaticHelperFunction.h"

FNPS_ClientPawnPrediction::FNPS_ClientPawnPrediction()
	: ClientInputBuffers(20)
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

const FSavedInput& FNPS_ClientPawnPrediction::GetSavedInput(uint32 ClientTick, bool UseNearestIfOutOfBound /*=true*/) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(
		ClientInputBuffersStartTickIndex, ClientTick, OutArrayIndex);

	if (UseNearestIfOutOfBound)
	{
		if (OutArrayIndex < 0)
		{
			OutArrayIndex = 0;
		}
		else if (OutArrayIndex >= ClientInputBuffers.Num())
		{
			OutArrayIndex = ClientInputBuffers.Num() - 1;
		}
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

	if (HasUnacknowledgedInput())
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
	else
	{
		int32 OutArrayIndex;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex(
			OldestUnacknowledgedInput, ClientTickIndex, OutArrayIndex);

		if (OutArrayIndex > 0.5f*TNumericLimits<int32>::Max())
		{
			// Prevent overflow problem if 
			OldestUnacknowledgedInput = ClientTickIndex;
		}
	}
}

