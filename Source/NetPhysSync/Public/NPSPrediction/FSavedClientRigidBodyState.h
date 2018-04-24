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

	static const FSavedClientRigidBodyState InvalidState;

	void SaveReplicatedRigidBodyState(const physx::PxRigidDynamic* const RigidDynamic);
	void SaveReplicatedRigidBodyState(const FReplicatedRigidBodyState& ReplicatedState);
	void GetReplicatedRigidBodyState(physx::PxRigidDynamic* const RigidDynamic) const;
	FORCEINLINE const FReplicatedRigidBodyState& GetReplicatedRigidBodyState() const;

	float CalculateSumDiffSqrtError(const FReplicatedRigidBodyState& OtherReplicatedState) const;
	float CalculateSumDiffSqrtError(const FSavedClientRigidBodyState& Other) const;

	/**
	 * If create by default constructor, ReplicatedState is not valid.
	 */
	FORCEINLINE bool IsReplicatedStateValid() const
	{
		return bIsReplicatedStateValid;
	}


private:
	bool bIsReplicatedStateValid;
	FReplicatedRigidBodyState ReplicatedRigidBodyStateState;

};
