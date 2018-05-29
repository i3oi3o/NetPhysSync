// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/**
 * Describes how to search for replay index.
 */
UENUM()
enum class EGetReplayIdxMethod : uint8
{
	SEARCH_OLDEST UMETA(DisplayName="Search oldest replay idx."),
	SEARCH_NEWEST UMETA(DisplayName="Search newest replay idx."),
};

