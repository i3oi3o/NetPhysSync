// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

class USceneComponent;
struct FVisualUpdateParam;
struct FReplayStartParam;
struct FReplayEndParam;

namespace physx
{
	class PxRigidDynamic;
}

/**
 * 
 */
class NETPHYSSYNC_API FVisualSmoothImplBase
{
public:
	FVisualSmoothImplBase();
	FVisualSmoothImplBase(TWeakObjectPtr<USceneComponent> SmoothTargetParam);
	virtual ~FVisualSmoothImplBase();

	virtual bool IsValid() const;
	virtual bool IsFinishYet() const;
	virtual void OnReplayStart
	(
		const FReplayStartParam& ReplayStartParam,
		const physx::PxRigidDynamic* RigidDynamic
	);

	virtual void OnReplayEnd
	(
		const FReplayEndParam& ReplayEndParam,
		const physx::PxRigidDynamic* RigidDynamic
	);

	virtual void VisualSmoothUpdate
	(
		const FVisualUpdateParam& Param
	);

protected:
	FORCEINLINE TWeakObjectPtr<USceneComponent> GetSmoothTarget() const
	{
		return SmoothTarget;
	}

	/*
	* Will sync if root is rigid body.
	*/
	void SyncVisualWithRootRigidBody();
private:
	TWeakObjectPtr<USceneComponent> SmoothTarget;
};
