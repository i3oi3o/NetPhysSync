// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FSavedInput.h"

FSavedInput::FSavedInput(FVector TagetSpeedParam)
	: TargetWorldSpeed(TagetSpeedParam)
{
}

FSavedInput::~FSavedInput()
{
}

float FSavedInput::CalculatedSumDiffSqrtError(const FSavedInput& Other) const
{
	return (TargetWorldSpeed - Other.TargetWorldSpeed).SizeSquared();
}

