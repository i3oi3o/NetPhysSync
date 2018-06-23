#include "AutomationTest.h"
#include "FTickSyncPoint.h"
#include "FOnNewSyncPointInfo.h"
#include "FReplayStartParam.h"
#include "Engine/EngineTypes.h"
// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSyncPointTest, "NetPhysSync.NetSyncManagement.SyncPointTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FSyncPointTest::RunTest(const FString& Parameters)
{
	const uint32 ClientTickSyncPoint = 7U;
	const uint32 ServerTickSyncPoint = 15U;
	FTickSyncPoint SyncPoint(ClientTickSyncPoint, ServerTickSyncPoint);

	TestEqual
	(
		TEXT("Test server syncpoint calculation."), 
		ClientTickSyncPoint+3U,
		SyncPoint.ServerTick2ClientTick(ServerTickSyncPoint+3U)
	);

	TestEqual
	(
		TEXT("Test client syncpoint calculation."),
		ServerTickSyncPoint+4U,
		SyncPoint.ClientTick2ServerTick(ClientTickSyncPoint+4U)
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShiftReplayPredictionTest, "NetPhysSync.NetSyncManagement.CalculateReplayShiftPredictionTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FShiftReplayPredictionTest::RunTest(const FString& Parameters)
{
	FTickSyncPoint OldSyncPoint( 0U, 4U );
	FTickSyncPoint NewSyncPoint( 7U, 15U);
	
	
	FOnNewSyncPointInfo OnNewSyncPointInfo( OldSyncPoint, NewSyncPoint);


	TestEqual
	(
		TEXT("Test shift amount for replay prediction."),
		OnNewSyncPointInfo.ShiftClientTickAmountForReplayPrediction,
		-4
	);

	FOnNewSyncPointInfo OnNoNewSyncPointInfo( NewSyncPoint, NewSyncPoint);

	TestEqual
	(
		TEXT("Shouldn't have any shift amount for no new sync point."),
		OnNoNewSyncPointInfo.ShiftClientTickAmountForReplayPrediction,
		0
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReplayStartParamTest, "NetPhysSync.NetSyncManagement.ReplayStartParam", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FReplayStartParamTest::RunTest(const FString& Parameters)
{
	FReplayStartParam TestStartParam
	(
		nullptr,
		EPhysicsSceneType::PST_Sync,
		FOnNewSyncPointInfo(FTickSyncPoint(),FTickSyncPoint()),
		0 - 5U,
		0U
	);

	if (TestStartParam.GetReplayNum() != 5)
	{
		AddError(TEXT("Calculate replay number count wrong."));
	}

	if (TestStartParam.IsReplayIntoFuture())
	{
		AddError(TEXT("Normal replay return replay into future."));
	}

	FReplayStartParam TestStartParam1
	(
		nullptr,
		EPhysicsSceneType::PST_Sync,
		FOnNewSyncPointInfo(FTickSyncPoint(), FTickSyncPoint()),
		0,
		0- 5U
	);

	if (TestStartParam1.GetReplayNum() != 0)
	{
		AddError(TEXT("Replay into future shouldn't have any replay nuumber count."));
	}

	if (!TestStartParam1.IsReplayIntoFuture())
	{
		AddError(TEXT("Why isn't this replay into future?"));
	}

	return true;
}