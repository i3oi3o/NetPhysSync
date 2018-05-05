// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IOnReplayEnd.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UOnReplayEnd : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NETPHYSSYNC_API IOnReplayEnd
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void OnReplayEnd() PURE_VIRTUAL(IOnReplayEnd::OnReplayEnd, );
	
};
