// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FReplicatedRigidBodyState.h"
#include "PxRigidDynamic.h"
#include "PhysXPublic.h"

using namespace physx;

FReplicatedRigidBodyState::FReplicatedRigidBodyState(PxRigidDynamic* const RigidDynamic)
	: LinearVelocity(P2UVector(RigidDynamic->getLinearVelocity()))
	, LinearAngularVelocity(P2UVector(RigidDynamic->getAngularVelocity()))
	, WorldPos(P2UVector(RigidDynamic->getGlobalPose().p))
	, WorldRotation(P2UQuat(RigidDynamic->getGlobalPose().q))
{
	
}

FReplicatedRigidBodyState::~FReplicatedRigidBodyState()
{
}
