// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FTickSyncPoint.h"

/**
 * 
 */
struct NETPHYSSYNC_API FOnNewSyncPointInfo
{
public:
	FOnNewSyncPointInfo
	(
		const FTickSyncPoint& OldSyncPointParam,
		const FTickSyncPoint& NewSyncPointParam
	);
	~FOnNewSyncPointInfo();


	const FTickSyncPoint OldSyncPoint;
	const FTickSyncPoint NewSyncPoint;
	const uint32 ShiftClientTickAmountForReplayPrediction;
};
