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

	FReplicatedRigidBodyState(physx::PxRigidDynamic* const RigidDynamic);
	~FReplicatedRigidBodyState();

	const FVector& GetLinearVelocity() { return LinearVelocity; }
	const FVector& GetLinearAngularVelocity() { return LinearAngularVelocity;}
	const FVector& GetWorldPos() { return WorldPos; }
	const FQuat& GetWorldRotation() { return WorldRotation; }

private:
	UPROPERTY()
	FVector LinearVelocity;
	UPROPERTY()
	FVector LinearAngularVelocity;
	UPROPERTY()
	FVector WorldPos;
	UPROPERTY()
	FQuat WorldRotation;
};
