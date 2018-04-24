// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FReplicatedRigidBodyState.generated.h"

namespace physx
{
	class PxRigidDynamic;
}

/**
 * 
 */
USTRUCT()
struct NETPHYSSYNC_API FReplicatedRigidBodyState
{
	GENERATED_BODY()
public:
	FReplicatedRigidBodyState() 
		: WorldPos()
		, WorldRotation(EForceInit::ForceInit)
		, LinearVelocity()
		, LinearAngularVelocity()
		, bIsSleep(false)
	{

	}

	/**
	 * Constructor for testing.
	 */
	FReplicatedRigidBodyState
	(
		FVector WorldPosParam,
		FQuat WorldRotationParam,
		FVector LinearVelocityParam,
		FVector LinearAngularVelocityParam,
		bool bIsSleepParam
	) 
		: WorldPos(WorldPosParam)
		, WorldRotation(WorldRotationParam)
		, LinearVelocity(LinearVelocityParam)
		, LinearAngularVelocity(LinearAngularVelocityParam)
		, bIsSleep(bIsSleepParam)
	{

	}

	FReplicatedRigidBodyState(const physx::PxRigidDynamic* const RigidDynamic);
	~FReplicatedRigidBodyState();

	FORCEINLINE const FVector& GetLinearVelocity() const { return LinearVelocity; }
	FORCEINLINE const FVector& GetLinearAngularVelocity() const { return LinearAngularVelocity;}
	FORCEINLINE const FVector& GetWorldPos() const { return WorldPos; }
	FORCEINLINE const FQuat& GetWorldRotation() const { return WorldRotation; }
	FORCEINLINE bool IsSleep() const { return bIsSleep; }
	void RetrivedRigidBodyState(physx::PxRigidDynamic* const RigidDynamic) const;
	float CalculateSumDiffSqrError(const FReplicatedRigidBodyState& Other) const;

private:
	UPROPERTY()
	FVector WorldPos;
	UPROPERTY()
	FQuat WorldRotation;
	UPROPERTY()
	FVector LinearVelocity;
	UPROPERTY()
	FVector LinearAngularVelocity;
	UPROPERTY()
	bool bIsSleep;
};
