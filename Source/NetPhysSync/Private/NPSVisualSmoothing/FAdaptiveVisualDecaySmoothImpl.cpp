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
			GetSmoothTarget()->RelativeLocation == FVector::ZeroVector &&
			GetSmoothTarget()->RelativeRotation == FRotator::ZeroRotator
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
		const FTransform& FT = GetSmoothTarget()->GetComponentTransform();
		CachReplayVisualPos = FT.GetLocation();
		CachReplayVisualQuat = FT.GetRotation();
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
		//SyncVisualWithRootRigidBody();
		GetSmoothTarget()->SetWorldLocationAndRotation
		(
			CachReplayVisualPos,
			CachReplayVisualQuat,
			false, nullptr /*FHitResult* */,
			ETeleportType::TeleportPhysics
		);
	}
}

void FAdaptiveVisualDecaySmoothImpl::VisualSmoothUpdate
(
	const FVisualUpdateParam& Param
)
{
	if (IsValid())
	{
		FVector RelativeLocation = GetSmoothTarget()->RelativeLocation;
		FQuat RelativeRotation = FQuat(GetSmoothTarget()->RelativeRotation);
		if (AdaptiveVisualDecayInfo.CanSnapPos(RelativeLocation, FVector::ZeroVector))
		{
			RelativeLocation = FVector::ZeroVector;
		}
		else
		{
			float DecayRate = AdaptiveVisualDecayInfo.GetDecayRate
			(RelativeLocation, FVector::ZeroVector);
			RelativeLocation = FMath::Lerp(RelativeLocation,
				FVector::ZeroVector, DecayRate*Param.GameFrameDeltaTime);
		}

		if (AdaptiveVisualDecayInfo.CanSnapRot(RelativeRotation, FQuat::Identity))
		{
			RelativeRotation = FQuat::Identity;
		}
		else
		{
			float DecayRate = AdaptiveVisualDecayInfo.GetDecayRate
			(RelativeRotation, FQuat::Identity);
			RelativeRotation = FQuat::Slerp(RelativeRotation,
				FQuat::Identity, DecayRate*Param.GameFrameDeltaTime);
			
		}

		GetSmoothTarget()->SetRelativeLocationAndRotation
		(
			RelativeLocation,
			RelativeRotation,
			false /*Sweep*/,
			nullptr /*FHitResult*/,
			ETeleportType::TeleportPhysics
		);
	}
}
