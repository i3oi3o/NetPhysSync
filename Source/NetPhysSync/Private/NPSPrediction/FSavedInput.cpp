// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FSavedInput.h"



FSavedInput::FSavedInput(FVector TagetSpeedParam)
	: TargetWorldSpeed(TagetSpeedParam)
{
}

FSavedInput::~FSavedInput()
{
}

const FSavedInput FSavedInput::EmptyInput = FSavedInput();

float FSavedInput::CalculatedSumDiffSqrtError(const FSavedInput& Other) const
{
	return (TargetWorldSpeed - Other.TargetWorldSpeed).SizeSquared();
}

FString FSavedInput::ToString() const
{
	return FString::Printf(TEXT("{ TargetWorldSpeed:{ %s } } "), 
		*TargetWorldSpeed.ToString());
}

bool FSavedInput::IsEmptyInput() const
{
	return TargetWorldSpeed == FVector::ZeroVector;
}

