// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FReplicatedRigidBodyState.h"
#include "PxRigidDynamic.h"
#include "PhysXPublic.h"

using namespace physx;

FReplicatedRigidBodyState::FReplicatedRigidBodyState(const PxRigidDynamic* const RigidDynamic)
	: LinearVelocity(P2UVector(RigidDynamic->getLinearVelocity()))
	, LinearAngularVelocity(P2UVector(RigidDynamic->getAngularVelocity()))
	, WorldPos(P2UVector(RigidDynamic->getGlobalPose().p))
	, WorldRotation(P2UQuat(RigidDynamic->getGlobalPose().q))
	, bIsSleep(RigidDynamic->isSleeping())
{
	
}

FReplicatedRigidBodyState::~FReplicatedRigidBodyState()
{
}

void FReplicatedRigidBodyState::RetrivedRigidBodyState(PxRigidDynamic* const RigidDynamic) const
{
	const PxVec3 PxWorldPos = U2PVector(WorldPos);
	const PxQuat PxWorldRot = U2PQuat(WorldRotation);
	RigidDynamic->setGlobalPose(PxTransform(PxWorldPos, PxWorldRot));

	if (bIsSleep)
	{
		RigidDynamic->putToSleep();
	}
	else
	{
		const PxVec3 PxLinearVelocity = U2PVector(LinearVelocity);
		const PxVec3 PxLinearAngularVelocity = U2PVector(LinearAngularVelocity);
		RigidDynamic->setAngularVelocity(PxLinearAngularVelocity);
		RigidDynamic->setLinearVelocity(PxLinearVelocity);
	}
}

float FReplicatedRigidBodyState::CalculateSumDiffSqrError
(
	const FReplicatedRigidBodyState& Other
) const
{
	float SumSqrtError = 0;
	SumSqrtError += (WorldPos - Other.WorldPos).SizeSquared();
	// Scale Quaternion error by 10000 because physx try to maintain unit quaternion.
	SumSqrtError += 10000*(WorldRotation - Other.WorldRotation).SizeSquared();
	SumSqrtError += (LinearVelocity - Other.LinearVelocity).SizeSquared();
	SumSqrtError += (LinearAngularVelocity - Other.LinearAngularVelocity).SizeSquared();
	return SumSqrtError;
}
