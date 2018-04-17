// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FReplicatedRigidBodyState.h"



/**
 * This is mutable.
 */
struct NETPHYSSYNC_API FSavedClientRigidBodyState
{
public:
	FSavedClientRigidBodyState();
	FSavedClientRigidBodyState(const physx::PxRigidDynamic* const RigidDynamic);
	FSavedClientRigidBodyState(const FReplicatedRigidBodyState& ReplicatedStateParam);
	~FSavedClientRigidBodyState();

	void SaveRigidBodyState(const physx::PxRigidDynamic* const RigidDynamic);
	void SaveReplicatedRigidBodyState(const FReplicatedRigidBodyState& ReplicatedState);
	void RetriveRigidBodyState(physx::PxRigidDynamic* const RigidDynamic) const;
	/**
	 * If create by default constructor, ReplicatedState is not valid.
	 */
	const bool IsReplicatedStateValid() const;


private:
	bool bIsReplicatedStateValid;
	FReplicatedRigidBodyState ReplicatedRigidBodyStateState;

};
