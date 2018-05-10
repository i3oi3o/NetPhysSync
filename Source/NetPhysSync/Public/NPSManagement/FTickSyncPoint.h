// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct NETPHYSSYNC_API FTickSyncPoint
{
public:
	FTickSyncPoint();

	FTickSyncPoint
	(
		const uint32 ClientTickSyncPointParam,
		const uint32 ServerTickSyncPointParam
	);
	~FTickSyncPoint();

	FORCEINLINE uint32 GetClientTickSyncPoint() const
	{
		return ClientTickSyncPoint;
	}

	FORCEINLINE uint32 GetServerTickSyncPoint() const
	{
		return ServerTickSyncPoint;
	}

	FORCEINLINE bool IsValid() const
	{
		return bIsValid;
	}

	uint32 ServerTick2ClientTick(uint32 ServerTick) const;
	uint32 ClientTick2ServerTick(uint32 ClientTick) const;

	FString ToString() const;

private:
	uint32 ClientTickSyncPoint;
	uint32 ServerTickSyncPoint;
	bool bIsValid;
};
