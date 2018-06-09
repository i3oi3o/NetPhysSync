// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FAdaptiveVisualDecaySmoothImpl.h"
#include "Components/SceneComponent.h"
#include "PxRigidDynamic.h"
#include "PhysXPublic.h"
#include "FVisualUpdateParam.h"

using namespace physx;

FAdaptiveVisualDecaySmoothImpl::FAdaptiveVisualDecaySmoothImpl()
{
}

FAdaptiveVisualDecaySmoothImpl::FAdaptiveVisualDecaySmoothImpl
(
	TWeakObjectPtr<USceneComponent> SmoothTargetParam, 
	FAdaptiveVisualDecayInfo AdaptiveVisualDecayInfoParam
)
	: Super(SmoothTargetParam)
	, AdaptiveVisualDecayInfo(AdaptiveVisualDecayInfoParam)
{
	AdaptiveVisualDecayInfo.CachDominator();
}

FAdaptiveVisualDecaySmoothImpl::~FAdaptiveVisualDecaySmoothImpl()
{
}

bool FAdaptiveVisualDecaySmoothImpl::IsFinishYet() const
{
	return !IsValid() ||
		(
			RelativePos == FVector::ZeroVector &&
			RelativeQuat == FQuat::Identity
		);
}

void FAdaptiveVisualDecaySmoothImpl::OnReplayStart
(
	const FReplayStartParam& ReplayStartParam, 
	const PxRigidDynamic* RigidDynamic
)
{
	if (IsValid())
	{
		// Detach from root.
		if (!GetSmoothTarget()->bAbsoluteLocation || 
			!GetSmoothTarget()->bAbsoluteRotation)
		{
			const FTransform WorldT = GetSmoothTarget()->GetComponentTransform();
			GetSmoothTarget()->SetAbsolute(true, true);
			// Somehow, The implementation in USceneComponent::SetAbsolute 
			// doesn't update FTransfrom properly.
			GetSmoothTarget()->SetWorldTransform(WorldT);
		}
	}
}

void FAdaptiveVisualDecaySmoothImpl::OnReplayEnd
(
	const FReplayEndParam& ReplayEndParam, 
	const PxRigidDynamic* RigidDynamic
)
{
	if (IsValid())
	{
		SyncRootWithRigidBody();
		const USceneComponent* Root = GetSmoothTarget()->GetAttachmentRoot();
		const FTransform& VT = GetSmoothTarget()->GetComponentTransform();
		const FTransform& RT = Root->GetComponentTransform();
		RelativePos = VT.GetLocation()- RT.GetLocation();
		RelativeQuat = RT.GetRotation().Inverse()*VT.GetRotation();
	}
}

void FAdaptiveVisualDecaySmoothImpl::VisualSmoothUpdate
(
	const FVisualUpdateParam& Param
)
{
	if (IsValid())
	{
		if (AdaptiveVisualDecayInfo.CanSnapPos(RelativePos, FVector::ZeroVector))
		{
			RelativePos = FVector::ZeroVector;
		}
		else
		{
			float DecayRate = AdaptiveVisualDecayInfo
				.GetDecayRate(RelativePos, FVector::ZeroVector);
			RelativePos = FMath::Lerp(RelativePos,
				FVector::ZeroVector, DecayRate*Param.GameFrameDeltaTime);
		}

		if (AdaptiveVisualDecayInfo.CanSnapRot(RelativeQuat, FQuat::Identity))
		{
			RelativeQuat = FQuat::Identity;
		}
		else
		{
			float DecayRate = AdaptiveVisualDecayInfo
				.GetDecayRate(RelativeQuat, FQuat::Identity);
			RelativeQuat = FQuat::Slerp(RelativeQuat,
				FQuat::Identity, DecayRate*Param.GameFrameDeltaTime);
		}

		if (IsFinishYet())
		{
			GetSmoothTarget()->SetAbsolute(false, false);
			GetSmoothTarget()->SetRelativeLocationAndRotation
			(
				FVector::ZeroVector,
				FQuat::Identity,
				false, nullptr, ETeleportType::TeleportPhysics
			);
		}
		else
		{
			const USceneComponent* Root = GetSmoothTarget()->GetAttachmentRoot();
			const FTransform& RT = Root->GetComponentToWorld();
			FVector SetWorldPos = RT.GetLocation() + RelativePos;
			FQuat SetWorldQuat = RT.GetRotation()*RelativeQuat;

			GetSmoothTarget()->SetWorldLocationAndRotation
			(
				SetWorldPos,
				SetWorldQuat,
				false,
				nullptr,
				ETeleportType::TeleportPhysics
			);
		}
	}
}
