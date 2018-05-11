// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FSavedInput.h"
#include "FAutonomousProxyInput.generated.h"

/**
 * 
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
	
	~FAutonomousProxyInput();

	FORCEINLINE uint32 GetArrayStartClientTickIndex() const
	{
		return ArrayStartClientTickIndex;
	}

	FORCEINLINE const TArray<FSavedInput>& GetArray() const
	{
		return InputArray;
	}

public:
	/**
	 * Currently for debug
	 */
	UPROPERTY()
	uint32 SendTickStamp;

private:
	UPROPERTY()
	TArray<FSavedInput> InputArray;
	UPROPERTY()
	uint32 ArrayStartClientTickIndex;

};
