// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ClientPawnPrediction.h"
#include "FNPS_StaticHelperFunction.h"

FNPS_ClientPawnPrediction::FNPS_ClientPawnPrediction()
	: ClientInputBuffers(20)
	, ClientInputBuffersStartTickIndex(0)
	, EmptySaveInput()
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
	FNPS_StaticHelperFunction::SetElementToBuffers
	(
		ClientInputBuffers, ToSave,
		ClientInputBuffersStartTickIndex, ClientTickIndex
	);
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
		return EmptySaveInput;
	}
}


bool FNPS_ClientPawnPrediction::HasUnacknowledgedInput() const
{
	return ClientInputBuffers.Num() > 0;
}

uint32 FNPS_ClientPawnPrediction::GetLastUnacknowledgeInputClientTickIndex() const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		LastUnacknowledgeInput,
		ClientInputBuffersStartTickIndex,
		OutArrayIndex
	);

	if (OutArrayIndex > 0)
	{
		return ClientInputBuffersStartTickIndex;
	}
	else
	{
		return LastUnacknowledgeInput;
	}
}

void FNPS_ClientPawnPrediction::ShiftBufferElementsToDifferentClientTick(int32 ShiftAmount)
{
	Super::ShiftBufferElementsToDifferentClientTick(ShiftAmount);
	ClientInputBuffersStartTickIndex += ShiftAmount;
	LastUnacknowledgeInput += ShiftAmount;
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
			LastUnacknowledgeInput = ClientInputBuffersStartTickIndex
				+ ClientInputBuffers.Num() - 1;
			ClientInputBuffers.Empty();
		}
		else
		{
			LastUnacknowledgeInput = ClientTickIndex;
		}
	}
}

