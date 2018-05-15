// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "AutomationTest.h"
#include "FAdaptiveVisualDecayInfo.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAdaptiveVisualDecayInfoTest, "NetPhysSync.VisualSmooth.AdaptiveDecayInfo", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FAdaptiveVisualDecayInfoTest::RunTest(const FString& Parameters)
{
	FAdaptiveVisualDecayInfo AdaptiveDecayInfo
	(
		3.0f, 9.0f,
		0.01f, 0.125f, 100.0f,
		0.004363323f /*0.25 degrees*/, 0.0174533f/*1 degrees*/, 0.0872665f /*5 degrees*/
	);

	FRotator CurrentRot(10.0f, 10.0f, 10.0f);
	FRotator SnapableErrorRot(0.5f*AdaptiveDecayInfo.GetSnapDegreeThreshold(), 
		0.0f, 0.0f);
	FRotator ErrorRot(0.0f, 
		0.5f*(AdaptiveDecayInfo.GetSmallDegreeThreshold() + 
			AdaptiveDecayInfo.GetBigDegreeThreshold()), 
		0.0f);

	FVector CurrentPos(10.0f, 10.0f, 10.0f);
	FVector SnapableErrorPos(0.5f*FMath::Sqrt(AdaptiveDecayInfo.GetSnapPosSqrtThreshold()), 
		0.0f, 0.0f);
	FVector ErrorPos(0.0f, 
		FMath::Sqrt(0.5f*(AdaptiveDecayInfo.GetSmallPosSqrtThreshold()+
			AdaptiveDecayInfo.GetBigPosSqrtThreshold())), 
		0.0f);



	TestEqual
	(
		TEXT("Check snap rotation"),
		AdaptiveDecayInfo.CanSnapRot
		(
			FQuat(CurrentRot),
			FQuat(CurrentRot + SnapableErrorRot)
		),
		true
	);

	TestEqual
	(
		TEXT("Check snap position"),
		AdaptiveDecayInfo.CanSnapPos
		(
			CurrentPos,
			CurrentPos + SnapableErrorPos
		),
		true
	);

	float ToTestDecay = 0.5f*(AdaptiveDecayInfo.GetDecayRateSmallDiff() +
		AdaptiveDecayInfo.GetDecayRateBigDiff());

	TestEqual
	(
		TEXT("Check pos decay rate"),
		AdaptiveDecayInfo.GetDecayRate
		(
			CurrentPos,
			CurrentPos + ErrorPos
		),
		ToTestDecay, 0.01f
	);

	TestEqual
	(
		TEXT("Check rot decay rate"),
		AdaptiveDecayInfo.GetDecayRate
		(
			FQuat(CurrentRot),
			FQuat(CurrentRot+ErrorRot)
		),
		ToTestDecay, 0.01f
	);

	return true;
}