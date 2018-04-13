// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FSavedInput.h"

FSavedInput::FSavedInput(FVector TagetSpeedParam, uint8 TickCount)
	: TargetSpeed(TagetSpeedParam)
	, TickCount(TickCount)
{
}

FSavedInput::~FSavedInput()
{
}

