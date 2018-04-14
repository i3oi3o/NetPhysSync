// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FSavedClientRigidBodyState.h"

FSavedClientRigidBodyState::FSavedClientRigidBodyState()
	: bIsReplicatedStateValid(false)
{
}

FSavedClientRigidBodyState::FSavedClientRigidBodyState(const physx::PxRigidDynamic* const RigidDynamic)
	: bIsReplicatedStateValid(true)
	, ReplicatedRigidBodyStateState(RigidDynamic)
{

}

FSavedClientRigidBodyState::FSavedClientRigidBodyState(const FReplicatedRigidBodyState& ReplicatedStateParam)
	: bIsReplicatedStateValid(true)
	, ReplicatedRigidBodyStateState(ReplicatedStateParam)
{

}

FSavedClientRigidBodyState::~FSavedClientRigidBodyState()
{
}

void FSavedClientRigidBodyState::SaveRigidBodyState(const physx::PxRigidDynamic* const RigidDynamic)
{
	bIsReplicatedStateValid = true;
	this->ReplicatedRigidBodyStateState = FReplicatedRigidBodyState(RigidDynamic);
}

void FSavedClientRigidBodyState::SaveReplicatedRigidBodyState(const FReplicatedRigidBodyState& ReplicatedState)
{
	this->bIsReplicatedStateValid = true;
	this->ReplicatedRigidBodyStateState = ReplicatedState;
}

void FSavedClientRigidBodyState::RetriveRigidBodyState(physx::PxRigidDynamic* const RigidDynamic) const
{
	if (bIsReplicatedStateValid)
	{
		ReplicatedRigidBodyStateState.RetrivedRigidBodyState(RigidDynamic);
	}
}

const bool& FSavedClientRigidBodyState::IsReplicatedStateValid()
{
	return bIsReplicatedStateValid;
}
