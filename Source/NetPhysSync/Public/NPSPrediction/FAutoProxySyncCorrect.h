// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FReplicatedRigidBodyState.h"
#include "FTickSyncPoint.h"
#include "FAutoProxySyncCorrect.generated.h"

/**
 * FAutonomousProxySyncClientTickCorrection.
 */
USTRUCT()
struct NETPHYSSYNC_API FAutoProxySyncCorrect
{

	GENERATED_BODY()

public:
	FAutoProxySyncCorrect();

	FAutoProxySyncCorrect
	(
		const FReplicatedRigidBodyState& ReplicateStateParam,
		uint32 SyncClientTickParam,
		uint32 SyncServerTickParam
	);

	FAutoProxySyncCorrect
	(
		const FReplicatedRigidBodyState& ReplicateStateParam,
		uint32 SyncClientTickParam,
		uint32 SyncServerTickParam,
		uint32 LastProcessedClientInputTick
	);

	~FAutoProxySyncCorrect();

	FORCEINLINE const FReplicatedRigidBodyState& GetRigidBodyState() const
	{
		return ReplicateState;
	}

	FORCEINLINE uint32 GetSyncClientTick() const
	{
		return SyncClientTick;
	}

	FORCEINLINE uint32 GetSyncServerTick() const
	{
		return SyncServerTick;
	}

	FORCEINLINE FTickSyncPoint CreateTickSyncPoint() const
	{
		return FTickSyncPoint(SyncClientTick, SyncServerTick);
	}

	bool TryGetLastProcessedClientInputTick(uint32& OutTick) const;

private:
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	FReplicatedRigidBodyState ReplicateState;
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	uint32 SyncClientTick;
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	uint32 SyncServerTick;
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	int32 OffsetClientTickToGetLastProcessedInputTick;
};
