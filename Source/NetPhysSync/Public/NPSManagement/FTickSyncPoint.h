// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct NETPHYSSYNC_API FTickSyncPoint
{
public:
	FTickSyncPoint
	(
		const uint32 ClientTickSyncPointParam,
		const uint32 ServerTickSyncPointParam
	);
	~FTickSyncPoint();

	const uint32 ClientTickSyncPoint;
	const uint32 ServerTickSyncPoint;

	uint32 ServerTick2ClientTick(uint32 ServerTick) const;
	uint32 ClientTick2ServerTick(uint32 ClientTick) const;
};
