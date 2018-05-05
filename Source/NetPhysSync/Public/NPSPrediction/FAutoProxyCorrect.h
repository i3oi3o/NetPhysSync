// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FReplicatedRigidBodyState.h"
#include "FAutoProxyCorrect.generated.h"

/**
 * 
 */
USTRUCT()
struct NETPHYSSYNC_API FAutoProxyCorrect
{
	GENERATED_BODY()

public:
	FAutoProxyCorrect();
	FAutoProxyCorrect
	(
		const FReplicatedRigidBodyState& ReplicatedRigidBodyStateParam,
		uint32 ServerTickParam
	);
	~FAutoProxyCorrect();

	FORCEINLINE const FReplicatedRigidBodyState& GetRigidBodyState() const
	{
		return ReplicatedRigidBodyState;
	}

	FORCEINLINE uint32 GetServerTick() const
	{
		return ServerTick;
	}

private:
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	FReplicatedRigidBodyState ReplicatedRigidBodyState;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	uint32 ServerTick;
};
