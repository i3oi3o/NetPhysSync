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
	
	static const FSavedInput EmptyInput;

	const FVector& GetTargetSpeed() const { return TargetWorldSpeed; }

	float CalculatedSumDiffSqrtError(const FSavedInput& Other) const;

	FString ToString() const;

	/**
	 * Use this to mark the end of input buffer that there is no more input.
	 */
	bool IsEmptyInput() const;

	bool operator==(const FSavedInput& Other) const;

	bool operator!=(const FSavedInput& Other) const;

private:
	UPROPERTY()
	FVector TargetWorldSpeed;
};
