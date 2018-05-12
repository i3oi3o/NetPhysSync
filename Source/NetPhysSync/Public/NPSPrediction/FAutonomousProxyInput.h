// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FSavedInput.h"
#include "FAutonomousProxyInput.generated.h"

/**
 * Implement using rule of five.
 */
USTRUCT()
struct NETPHYSSYNC_API FAutonomousProxyInput
{
	GENERATED_BODY()
public:
	
	FAutonomousProxyInput();

	FAutonomousProxyInput(const class FNPS_ClientPawnPrediction& ClientPawnPrediction);

	template<typename AllocatorType>
	FAutonomousProxyInput
	(
		TArray<FSavedInput, AllocatorType>& InputArrayParam, 
		uint32 ArrayStartClientTickIndexParam
	)
		: InputArray(InputArrayParam)
		, ArrayStartClientTickIndex(ArrayStartClientTickIndexParam)
	{
		
	}
	
	FAutonomousProxyInput(const FAutonomousProxyInput& Other) = default;
	FAutonomousProxyInput(FAutonomousProxyInput&& Other) = default;
	FAutonomousProxyInput& operator=(const FAutonomousProxyInput& Other) = default;
	FAutonomousProxyInput& operator=(FAutonomousProxyInput&& Other) = default;
	~FAutonomousProxyInput();

	FORCEINLINE uint32 GetArrayStartClientTickIndex() const
	{
		return ArrayStartClientTickIndex;
	}

	FORCEINLINE const TArray<FSavedInput>& GetArray() const
	{
		return InputArray;
	}

private:
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	TArray<FSavedInput> InputArray;
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	uint32 ArrayStartClientTickIndex;

};
