// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FSavedInput.generated.h"

/**
 * 
 */
USTRUCT()
struct NETPHYSSYNC_API FSavedInput
{
	GENERATED_BODY()
public:
	FSavedInput(): TickCount(0)
		, TargetSpeed()
	{
	
	}

	FSavedInput(FVector TargetSpeedParam, uint8 TickCount = 1);
	~FSavedInput();
	
	FVector GetTargetSpeed() const { return TargetSpeed; }
	uint8 GetTickCount() const { return TickCount; }

private:
	UPROPERTY()
	uint8 TickCount;
	UPROPERTY()
	FVector TargetSpeed;
};
