// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FAdaptiveVisualDecayInfo.generated.h"

/**
 * 
 */
USTRUCT()
struct NETPHYSSYNC_API FAdaptiveVisualDecayInfo
{
	GENERATED_BODY()

public:
	FAdaptiveVisualDecayInfo();
	FAdaptiveVisualDecayInfo
	(
		float DecayRateSmallDiffParam,
		float DecayRateBigDiffParam,
		float SnapPosSqrtThresholdParam,
		float SmallPosSqrtThresholdParam,
		float BigPosSqrtThresholdParam,
		float SnapRadianThresholdParam,
		float SmallRadianThresholdParam,
		float BigRadianThresholdParam
	);
	~FAdaptiveVisualDecayInfo();

	bool CanSnapPos(const FVector& A, const FVector& B) const;
	bool CanSnapRot(const FQuat& A, const FQuat& B) const;
	float GetDecayRate(const FVector& A, const FVector& B) const;
	float GetDecayRate(const FQuat& A, const FQuat& B) const;

	/**
	 * This is supposed to call in constructor but if this data is serialized in UPROPERTY(),
	 * We need to manually call this.
	 */
	void CachDominator();

	FORCEINLINE float GetDecayRateSmallDiff() const
	{
		return DecayRateSmallDiff;
	}

	FORCEINLINE float GetDecayRateBigDiff() const
	{
		return DecayRateBigDiff;
	}

	FORCEINLINE float GetSnapPosSqrtThreshold() const
	{
		return SnapPosSqrtThreshold;
	}

	FORCEINLINE float GetSmallPosSqrtThreshold() const
	{
		return SmallPosSqrtThreshold;
	}

	FORCEINLINE float GetBigPosSqrtThreshold() const
	{
		return BigPosSqrtThreshold;
	}

	FORCEINLINE float GetSnapRadianThreshold() const
	{
		return SnapRadianThreshold;
	}

	float GetSnapDegreeThreshold() const;

	FORCEINLINE float GetSmallRadianThreshold() const
	{
		return SmallRadianThreshold;
	}

	float GetSmallDegreeThreshold() const;

	FORCEINLINE float GetBigRadianThreshold() const
	{
		return BigRadianThreshold;
	}

	float GetBigDegreeThreshold() const;

private:
	UPROPERTY()
	float DecayRateSmallDiff;
	UPROPERTY()
	float DecayRateBigDiff;
	UPROPERTY()
	float SnapPosSqrtThreshold;
	UPROPERTY()
	float SmallPosSqrtThreshold;
	UPROPERTY()
	float BigPosSqrtThreshold;
	UPROPERTY()
	float SnapRadianThreshold;
	UPROPERTY()
	float SmallRadianThreshold;
	UPROPERTY()
	float BigRadianThreshold;

	bool bCachDenomiatorYet;

	/**
	 * Cache denominator for faster calculation.
	 * = 1/(BigPosSqrtThreshold - SmallPosSqrtThreshold)
	 */
	float CachPosLerpDenominator;
	/*
	* Cache denominator for faster calculation.
	* = 1/(BigRadianThreshold - SmallRaidianThreshold)
	*/
	float CachRaidanLerpDenominator;
};
