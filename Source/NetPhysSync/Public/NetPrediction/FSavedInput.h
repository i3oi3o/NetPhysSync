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
	FSavedInput(): 
		TargetWorldSpeed(0.0f, 0.0f, 0.0f)
	{
	
	}

	FSavedInput(FVector TargetSpeedParam);
	~FSavedInput();
	
	FVector GetTargetSpeed() const { return TargetWorldSpeed; }
	const FVector& GetConstTargetSpeedReference() const { return TargetWorldSpeed; }

private:
	UPROPERTY()
	FVector TargetWorldSpeed;
};
