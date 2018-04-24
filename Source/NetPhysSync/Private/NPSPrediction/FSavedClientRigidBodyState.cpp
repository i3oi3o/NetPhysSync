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

const FSavedClientRigidBodyState FSavedClientRigidBodyState::InvalidState = FSavedClientRigidBodyState();

void FSavedClientRigidBodyState::SaveReplicatedRigidBodyState(const physx::PxRigidDynamic* const RigidDynamic)
{
	bIsReplicatedStateValid = true;
	this->ReplicatedRigidBodyStateState = FReplicatedRigidBodyState(RigidDynamic);
}

void FSavedClientRigidBodyState::SaveReplicatedRigidBodyState(const FReplicatedRigidBodyState& ReplicatedState)
{
	this->bIsReplicatedStateValid = true;
	this->ReplicatedRigidBodyStateState = ReplicatedState;
}

void FSavedClientRigidBodyState::GetReplicatedRigidBodyState(physx::PxRigidDynamic* const RigidDynamic) const
{
	if (bIsReplicatedStateValid)
	{
		ReplicatedRigidBodyStateState.RetrivedRigidBodyState(RigidDynamic);
	}
}

FORCEINLINE const FReplicatedRigidBodyState& FSavedClientRigidBodyState::GetReplicatedRigidBodyState() const
{
	return ReplicatedRigidBodyStateState;
}

float FSavedClientRigidBodyState::CalculateSumDiffSqrtError(const FReplicatedRigidBodyState& OtherReplicatedState) const
{
	return ReplicatedRigidBodyStateState.CalculateSumDiffSqrError(OtherReplicatedState);
}

float FSavedClientRigidBodyState::CalculateSumDiffSqrtError(const FSavedClientRigidBodyState& Other) const
{
	return ReplicatedRigidBodyStateState.CalculateSumDiffSqrError(Other.ReplicatedRigidBodyStateState);
}

