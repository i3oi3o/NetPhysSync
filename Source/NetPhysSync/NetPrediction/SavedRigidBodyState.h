// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

namespace physx
{
	class PxRigidDynamic;
}

/**
 * 
 */
class NETPHYSSYNC_API FSavedRigidBodyState
{
public:
	FSavedRigidBodyState(physx::PxRigidDynamic* RigidDynamic);
	~FSavedRigidBodyState();

	void RestoreRigidBodyState(physx::PxRigidDynamic* RigidDynamic) const;

	void SetIsNeedReplay(bool bIsNeedReplayParam);

	bool GetIsNeedReplay() const;

	void SetIsPrediction(bool bIsPrediction);

	bool GetIsPrediction() const;

private:
	FVector LinearVelocity;
	FVector LinearAngularVelocity;
	FVector WorldPos;
	FQuat WorldRotation;
	float WakeupCounter;
	bool bSleep;
	bool bIsNeedReplay;
	bool bIsPrediction;
};
