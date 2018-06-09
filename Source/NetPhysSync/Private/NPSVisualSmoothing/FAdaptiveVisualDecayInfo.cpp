// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FAdaptiveVisualDecayInfo.h"

FAdaptiveVisualDecayInfo::FAdaptiveVisualDecayInfo()
	: bCachDenomiatorYet(false)
{
}

FAdaptiveVisualDecayInfo::FAdaptiveVisualDecayInfo
(
	float DecayRateSmallDiffParam, 
	float DecayRateBigDiffParam, 
	float SnapPosSqrtThresholdParam, 
	float SmallPosSqrtThresholdParam, 
	float BigPosSqrtThresholdParam, 
	float SnapRadianThresholdParam, 
	float SmallRadianThresholdParam, 
	float BigRadianThresholdParam
)
	: DecayRateSmallDiff(DecayRateSmallDiffParam)
	, DecayRateBigDiff(DecayRateBigDiffParam)
	, SnapPosSqrtThreshold(SnapPosSqrtThresholdParam)
	, SmallPosSqrtThreshold(SmallPosSqrtThresholdParam)
	, BigPosSqrtThreshold(BigPosSqrtThresholdParam)
	, SnapRadianThreshold(SnapRadianThresholdParam)
	, SmallRadianThreshold(SmallRadianThresholdParam)
	, BigRadianThreshold(BigRadianThresholdParam)
	, bCachDenomiatorYet(false)
	, CachPosLerpDenominator(0.0f)
	, CachRaidanLerpDenominator(0.0f)
{
	CachDominator();
}

FAdaptiveVisualDecayInfo::~FAdaptiveVisualDecayInfo()
{
}

bool FAdaptiveVisualDecayInfo::CanSnapPos(const FVector& A, const FVector& B) const
{
	float DiffSqrt = (A - B).SizeSquared();
	return DiffSqrt <= SnapPosSqrtThreshold;
}

bool FAdaptiveVisualDecayInfo::CanSnapRot(const FQuat& A, const FQuat& B) const
{
	float DiffRadian = A.AngularDistance(B);
	return FMath::Abs(DiffRadian) <= SnapRadianThreshold;
}

float FAdaptiveVisualDecayInfo::GetDecayRate(const FVector& A, const FVector& B) const
{
	ensureMsgf(bCachDenomiatorYet, TEXT("Please call CachDominator first."));
	float DiffSqrt = (A - B).SizeSquared();
	float LerpV = (DiffSqrt - SmallPosSqrtThreshold)*CachPosLerpDenominator;
	LerpV = FMath::Clamp(LerpV, 0.0f, 1.0f);
	return FMath::Lerp(DecayRateSmallDiff, DecayRateBigDiff, LerpV);
}

float FAdaptiveVisualDecayInfo::GetDecayRate(const FQuat& A, const FQuat& B) const
{
	ensureMsgf(bCachDenomiatorYet, TEXT("Please call CachDominator first."));
	float DiffRadian = A.AngularDistance(B);
	float LerpV = (DiffRadian - SmallRadianThreshold)*CachRaidanLerpDenominator;
	LerpV = FMath::Clamp(LerpV, 0.0f, 1.0f);
	return FMath::Lerp(DecayRateSmallDiff, DecayRateBigDiff, LerpV);
}

void FAdaptiveVisualDecayInfo::CachDominator()
{
	if (!bCachDenomiatorYet)
	{
		float DiffSqrtPos = BigPosSqrtThreshold - SmallPosSqrtThreshold;
		if (DiffSqrtPos <= 0)
		{
			CachPosLerpDenominator = 0;
		}
		else
		{
			CachPosLerpDenominator = 1.0f / DiffSqrtPos;
		}
		
		float DiffRadian = BigRadianThreshold - SmallRadianThreshold;
		if (DiffRadian <= 0)
		{
			CachRaidanLerpDenominator = 0;
		}
		else
		{
			CachRaidanLerpDenominator = 1.0f / DiffRadian;
		}
		bCachDenomiatorYet = true;
	}
}

float FAdaptiveVisualDecayInfo::GetSnapDegreeThreshold() const
{
	return FMath::RadiansToDegrees(SnapRadianThreshold);
}

float FAdaptiveVisualDecayInfo::GetSmallDegreeThreshold() const
{
	return FMath::RadiansToDegrees(SmallRadianThreshold);
}

float FAdaptiveVisualDecayInfo::GetBigDegreeThreshold() const
{
	return FMath::RadiansToDegrees(BigRadianThreshold);
}
