// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * Describes how to handle out of bound when query history buffer
 */
UENUM()
enum class EIdxOutOfRangeHandle : uint8
{
	PreventExceptionByDoNothingOrReturnEmptyInvalid UMETA(DisplayName="PreventException"),
	UseNearestIndex UMETA(DisplayName = "UseNearestIndex"),
	UseNearestIndexIfOutRangeFromEnd UMETA(DisplayName = "UseNearestIndexIfOutRangeFromEnd"),
	UseNearestIndexIfOutRangeFromBegin UMETA(DisplayName = "UseNearestIndexIfOutRangeFromBegin")
};

