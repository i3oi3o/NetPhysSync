// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ClientPawnPrediction.h"
#include "FNPS_StaticHelperFunction.h"

FNPS_ClientPawnPrediction::FNPS_ClientPawnPrediction()
	: ClientInputBuffers(20),
	ClientInputBuffersStartTickIndex(0)
{
}

FNPS_ClientPawnPrediction::~FNPS_ClientPawnPrediction()
{
}

void FNPS_ClientPawnPrediction::SaveInput(FVector TargetWorldSpeed, uint32 ClientTickIndex)
{
	FNPS_StaticHelperFunction::SetElementToBuffers
	(
		ClientInputBuffers, FSavedInput(TargetWorldSpeed),
		ClientInputBuffersStartTickIndex, ClientTickIndex
	);
}

FVector FNPS_ClientPawnPrediction::GetSavedInput(uint32 ClientTick) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(
		ClientInputBuffersStartTickIndex, ClientTick, OutArrayIndex);
	if (OutArrayIndex >= 0 && OutArrayIndex < ClientInputBuffers.Num())
	{
		return ClientInputBuffers[OutArrayIndex].GetConstTargetSpeedReference();
	}
	else
	{
		return FVector(0.0f, 0.0f, 0.0f);
	}
}


void FNPS_ClientPawnPrediction::ShiftStartBufferIndex(int32 ShiftAmount)
{
	Super::ShiftStartBufferIndex(ShiftAmount);
	ClientInputBuffersStartTickIndex += ShiftAmount;
}

bool FNPS_ClientPawnPrediction::HasClientInputBuffers() const
{
	return ClientInputBuffers.Num() > 0;
}
