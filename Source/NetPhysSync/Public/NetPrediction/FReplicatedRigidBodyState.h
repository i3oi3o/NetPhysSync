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
	FReplicatedRigidBodyState() : LinearVelocity()
		,LinearAngularVelocity()
		, WorldPos()
		, WorldRotation(EForceInit::ForceInit)
	{

	}

	FReplicatedRigidBodyState(const physx::PxRigidDynamic* const RigidDynamic);
	~FReplicatedRigidBodyState();

	const FVector& GetLinearVelocity() const { return LinearVelocity; }
	const FVector& GetLinearAngularVelocity() const { return LinearAngularVelocity;}
	const FVector& GetWorldPos() const { return WorldPos; }
	const FQuat& GetWorldRotation() const { return WorldRotation; }
	const bool& IsSleep() const { return bIsSleep; }
	void RetrivedRigidBodyState(physx::PxRigidDynamic* const RigidDynamic) const;

private:
	UPROPERTY()
	FVector LinearVelocity;
	UPROPERTY()
	FVector LinearAngularVelocity;
	UPROPERTY()
	FVector WorldPos;
	UPROPERTY()
	FQuat WorldRotation;
	UPROPERTY()
	bool bIsSleep;
};
