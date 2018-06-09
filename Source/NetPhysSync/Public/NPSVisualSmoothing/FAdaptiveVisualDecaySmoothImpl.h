// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FVisualSmoothImplBase.h"
#include "FAdaptiveVisualDecayInfo.h"

//namespace physx
//{
//	class PxRigidDynamic;
//}

/**
 * 
 */
class NETPHYSSYNC_API FAdaptiveVisualDecaySmoothImpl : 
	public FVisualSmoothImplBase 
{
	typedef FVisualSmoothImplBase Super;
public:
	FAdaptiveVisualDecaySmoothImpl();
	FAdaptiveVisualDecaySmoothImpl
	(
		TWeakObjectPtr<USceneComponent> SmoothTargetParam,
		FAdaptiveVisualDecayInfo AdaptiveVisualDecayInfoParam
	);
	~FAdaptiveVisualDecaySmoothImpl();

	virtual bool IsFinishYet() const override;
	virtual void OnReplayStart
	(
		const FReplayStartParam& ReplayStartParam,
		const physx::PxRigidDynamic* RigidDynamic
	) override;

	virtual void OnReplayEnd
	(
		const FReplayEndParam& ReplayEndParam,
		const physx::PxRigidDynamic* RigidDynamic
	) override;

	virtual void VisualSmoothUpdate(const FVisualUpdateParam& Param);

private:
	FAdaptiveVisualDecayInfo AdaptiveVisualDecayInfo;
	FVector RelativePos;
	FQuat RelativeQuat;
};
