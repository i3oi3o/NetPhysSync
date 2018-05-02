// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct NETPHYSSYNC_API FVisualUpdateParam
{
public:
	FVisualUpdateParam(const float GameFrameDeltaTimeParam);
	~FVisualUpdateParam();

	const float GameFrameDeltaTime;
};
