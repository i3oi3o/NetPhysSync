// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "SavedInput.h"

FSavedInput::FSavedInput(FVector TagetSpeedParam)
	: TargetSpeed(TagetSpeedParam)
{
}

FSavedInput::~FSavedInput()
{
}

uint8 FSavedInput::ReduceTickCount(uint8 ReduceAmount)
{
	if (TickCount > ReduceAmount)
	{
		TickCount -= ReduceAmount;
	}
	else
	{
		TickCount = 0;
	}
}
