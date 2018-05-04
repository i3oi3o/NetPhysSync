// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FOnNewSyncPointInfo.h"

/**
 * 
 */
class NETPHYSSYNC_API FOnReadReplicationParam
{
public:
	FOnReadReplicationParam(const FOnNewSyncPointInfo NewSyncPointInfoParam);
	~FOnReadReplicationParam();

	const FOnNewSyncPointInfo NewSyncPointInfo;
};
