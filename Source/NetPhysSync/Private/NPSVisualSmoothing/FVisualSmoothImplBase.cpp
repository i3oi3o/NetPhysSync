// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FVisualSmoothImplBase.h"
#include "INetPhysSyncParam.h"

FVisualSmoothImplBase::FVisualSmoothImplBase()
{
}

FVisualSmoothImplBase::FVisualSmoothImplBase
(
	TWeakObjectPtr<USceneComponent> SmoothTargetParam
)
	: SmoothTarget(SmoothTargetParam)
{

}

FVisualSmoothImplBase::~FVisualSmoothImplBase()
{

}

bool FVisualSmoothImplBase::IsValid() const
{
	return SmoothTarget != nullptr;
}

bool FVisualSmoothImplBase::IsFinishYet() const
{
	return true;
}

void FVisualSmoothImplBase::OnReplayStart(const FReplayStartParam& ReplayStartParam, const physx::PxRigidDynamic* RigidDynamic)
{

}

void FVisualSmoothImplBase::OnReplayEnd(const FReplayEndParam& ReplayEndParam, const physx::PxRigidDynamic* RigidDynamic)
{

}
