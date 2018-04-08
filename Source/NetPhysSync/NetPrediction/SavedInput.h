// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct NETPHYSSYNC_API FSavedInput
{
public:
	FSavedInput(FVector TargetSpeedParam);
	~FSavedInput();

	FVector GetTargetSpeed() const { return TargetSpeed; }
	uint8 GetTickCount() const { return TickCount; }
	uint8 ReduceTickCount(uint8 ReduceAmount);

private:
	uint8 TickCount;
	FVector TargetSpeed;
};
