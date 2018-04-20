// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.
#include "AutomationTest.h"
#include "NumericLimits.h"
#include "FNPS_StaticHelperFunction.h"
#include "FNPS_ClientActorPrediction.h"
#include "FSavedClientRigidBodyState.h"

void GenerateFakedStateFunction(FSavedClientRigidBodyState* InSaveClientStateArray, int Count)
{
	for (int i = 0; i < Count; ++i)
	{
		FReplicatedRigidBodyState (Tmp)
		(
			FVector(static_cast<float>(i + 1), 0.0f, 0.0f),
			FQuat(FRotator(0.0f, static_cast<float>(i + 1), 0.0f)),
			FVector(0.0f, static_cast<float>(i + 1), 0.0f),
			FVector(0.0f, 0.0f, static_cast<float>(i + 1)),
			false
		);

		InSaveClientStateArray[i] = FSavedClientRigidBodyState(Tmp);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUint32OverflowTest, "NetPhysSync.PredictBuffer.Uint32OperationTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FUint32OverflowTest::RunTest(const FString& Parameters)
{
	bool TestResult = true;
	uint32 x = 2;
	uint32 y = 4;
	uint32 z = 7;

	TestResult &= (5 == ((x - y)+ z));

	TestResult &= ((TNumericLimits<uint32>::Max() - x+1U) == (x - y));

	TestResult &= (x - (TNumericLimits<uint32>::Max() - x + 1U) == x+x);

	TestResult &= (TNumericLimits<uint32>::Max() +1U == 0U);

	TestResult &= (TNumericLimits<uint32>::Max() == 0U - 1U);

	return TestResult;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBufferTickOverflowTest, "NetPhysSync.PredictBuffer.IndexOperationTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FBufferTickOverflowTest::RunTest(const FString& Parameters)
{
	bool TestResult = true;
	
	int32 TestIndex;
	uint32 HalfUint32 = TNumericLimits<uint32>::Max() / 2;

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(TNumericLimits<uint32>::Max(), 0U, TestIndex);
	TestResult &= (TestIndex == 1);
	
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(TNumericLimits<uint32>::Max()-2U, 3U, TestIndex);
	TestResult &= (TestIndex == 6);

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(HalfUint32-5U, HalfUint32+5U, TestIndex);
	TestResult &= (TestIndex == 10);

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(HalfUint32, HalfUint32 + 5U, TestIndex);
	TestResult &= (TestIndex == 5);

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(0U, TNumericLimits<uint32>::Max(), TestIndex);
	TestResult &= (TestIndex == -1);

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(3U, TNumericLimits<uint32>::Max()-2U, TestIndex);
	TestResult &= (TestIndex == -6);

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(HalfUint32 + 5U, HalfUint32 - 5U, TestIndex);
	TestResult &= (TestIndex == -10);

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(HalfUint32 + 5U, HalfUint32, TestIndex);
	TestResult &= (TestIndex == -5);

	return TestResult;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientActorPredictionHasBufferYetTest, "NetPhysSync.PredictBuffer.ClientActorHasStateBuffer", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientActorPredictionHasBufferYetTest::RunTest(const FString& Parameters)
{
	uint32 FakeClientTick = 0 - 10U;

	FNPS_ClientActorPrediction ClientActorPrediction;
	TestEqual
	(
		TEXT("Test If Buffer is Empty."), 
		ClientActorPrediction.HasClientStateBufferYet(),
		false
	);

	FReplicatedRigidBodyState(DummyReplicatedState)
		(
			FVector(1.0f, 0.0f, 0.0f),
			FQuat(EForceInit::ForceInit),
			FVector(),
			FVector(),
			true
			);

	FSavedClientRigidBodyState DummyState(DummyReplicatedState);

	ClientActorPrediction.SaveRigidBodyState(DummyState, FakeClientTick);

	TestEqual
	(
		TEXT("Test If Buffer is not Empty"),
		ClientActorPrediction.HasClientStateBufferYet(),
		true
	);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientActorPredictionSaveAndGetTest, "NetPhysSync.PredictBuffer.ClientActorSaveAndGet", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientActorPredictionSaveAndGetTest::RunTest(const FString& Parameters)
{
	
	uint32 FakeClientTick = 0 - 5U;
	FNPS_ClientActorPrediction ClientActorPrediction;
	FSavedClientRigidBodyState GenerateClientRigidBodyStates[10];

	GenerateFakedStateFunction(GenerateClientRigidBodyStates, 10);

	FReplicatedRigidBodyState (DummyReplicatedState)
	(
		FVector(1.0f, 0.0f, 0.0f),
		FQuat(EForceInit::ForceInit),
		FVector(),
		FVector(),
		true
	);

	FSavedClientRigidBodyState DummyState(DummyReplicatedState);

	for (int32 i = 0; i < 15; ++i)
	{
		ClientActorPrediction.SaveRigidBodyState(DummyState, FakeClientTick);
		++FakeClientTick;
	}

	for (int32 i = 1; i <= 15; ++i)
	{
		float ErrorDiff = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick - i)
			.CalculatedSumDiffSqrtError(DummyReplicatedState);

		TestEqual(TEXT("Compare Dummy RigidBodyState in buffer."), ErrorDiff, 0.0f);
	}

	for (int32 i = 0; i < 10; ++i)
	{
		ClientActorPrediction.SaveRigidBodyState
		(
			GenerateClientRigidBodyStates[i], 
			FakeClientTick
		);
		++FakeClientTick;
	}

	for (int32 i = 1; i <= 20; ++i)
	{
		float ErrorDiff = 0.0f;
		if (i <= 10)
		{
			ErrorDiff = ClientActorPrediction
				.GetRigidBodyState(FakeClientTick - i)
				.CalculatedSumDiffSqrtError(GenerateClientRigidBodyStates[10-i]);
			TestEqual(TEXT("Compare Generated RigidBodyState in buffer."), ErrorDiff, 0.0f);
		}
		else
		{
			ErrorDiff = ClientActorPrediction
				.GetRigidBodyState(FakeClientTick - i)
				.CalculatedSumDiffSqrtError(DummyReplicatedState);
			TestEqual(TEXT("Compare Dummy2 RigidBodyState in buffer."), ErrorDiff, 0.0f);
		}
	}

	{
		/*Test Query Out of bound element*/

		float ErrorDiff = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick + 10)
			.CalculatedSumDiffSqrtError(GenerateClientRigidBodyStates[9]);

		TestEqual(TEXT("Comapre out of bound state from future using nearest."), ErrorDiff, 0.0f);

		ErrorDiff = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick - 60)
			.CalculatedSumDiffSqrtError(DummyReplicatedState);

		TestEqual(TEXT("Compare out of bound state from past using nearest."), ErrorDiff, 0.0f);


		bool bIsValid = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick + 10, false)
			.IsReplicatedStateValid();

		TestEqual(TEXT("Test out of bound state from future without using nearest."), bIsValid, false);

		bIsValid = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick - 60, false)
			.IsReplicatedStateValid();

		TestEqual(TEXT("Test out of bound state from past without using nearest."), bIsValid, false);
	}

	return true;

}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientActorPredictionShiftBufferTest, "NetPhysSync.PredictBuffer.ClientActorShiftBufferTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientActorPredictionShiftBufferTest::RunTest(const FString& Parameters)
{
	uint32 FakeClientTick = 0U - 2U;

	FNPS_ClientActorPrediction ClientActorPrediction;

	FSavedClientRigidBodyState GeneratedSaveState[30];

	GenerateFakedStateFunction(GeneratedSaveState, 30);

	for (int32 i = 0; i < 30; ++i)
	{
		ClientActorPrediction.SaveRigidBodyState(GeneratedSaveState[i], FakeClientTick);
		++FakeClientTick;
	}

	ClientActorPrediction.ShiftBufferElementsToDifferentClientTick(-5);

	uint32 LastStoreClientTickBeforeShifting = FakeClientTick - 1;
	uint32 ToQuertByShiftingTick = LastStoreClientTickBeforeShifting - 5U;

	for (int32 i = 29; i >= 10; --i)
	{
		float DiffError = ClientActorPrediction
			.GetRigidBodyState(ToQuertByShiftingTick + (i-29))
			.CalculatedSumDiffSqrtError(GeneratedSaveState[i]);

		TestEqual(TEXT("Compare Shift-to-Past Buffer Value"), DiffError, 0.0f);
	}

	ClientActorPrediction.ShiftBufferElementsToDifferentClientTick(10);

	ToQuertByShiftingTick = LastStoreClientTickBeforeShifting + 5U;

	for (int32 i = 29; i >= 10; --i)
	{
		float DiffError = ClientActorPrediction
			.GetRigidBodyState(ToQuertByShiftingTick + (i - 29))
			.CalculatedSumDiffSqrtError(GeneratedSaveState[i]);

		TestEqual(TEXT("Compare Shift-to-Future Buffer Value"), DiffError, 0.0f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientActorPredictionReplayTickTest, "NetPhysSync.PredictBuffer.GetReplayTickTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientActorPredictionReplayTickTest::RunTest(const FString& Parameters)
{
	uint32 FakeClientTick = 0U - 2U;
	FNPS_ClientActorPrediction ClientActorPrediction;
	FSavedClientRigidBodyState GeneratedState[10];
	uint32 ForQueryReplayTick;
	float DiffSqrtError = 0;

	GenerateFakedStateFunction(GeneratedState, 10);

	for (int32 i = 0; i < 10; ++i)
	{
		ClientActorPrediction.SaveRigidBodyState
		(
			GeneratedState[i], 
			FakeClientTick+i
		);
	}


#pragma region Correct Same State
	ClientActorPrediction.ServerCorrectState
	(
		GeneratedState[4].GetRigidBodyState(),
		FakeClientTick + 4
	);

	bool NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

	TestEqual(TEXT("Correct State is the same. Shouldn't need replay."), NeedReplay, false);
#pragma endregion

#pragma region Correct Slightly Different
	const FReplicatedRigidBodyState& ExistState = GeneratedState[4].GetRigidBodyState();

	FReplicatedRigidBodyState (SlightlyDifferent)
	(
		ExistState.GetWorldPos() + FVector(0.01f, 0.0f, 0.0f),
		ExistState.GetWorldRotation(),
		ExistState.GetLinearVelocity(),
		ExistState.GetLinearAngularVelocity(),
		ExistState.IsSleep()
	);

	ClientActorPrediction.ServerCorrectState
	(
		SlightlyDifferent,
		FakeClientTick + 4
	);

	NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

	TestEqual(TEXT("Correct State is the same. Shouldn't need replay."), NeedReplay, false);
#pragma endregion

#pragma region FakeClientTick + 4
	ClientActorPrediction.ServerCorrectState
	(
		GeneratedState[5].GetRigidBodyState(),
		FakeClientTick + 4
	);

	NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

	TestEqual(TEXT("Need replay at FakeClientTick+4"), NeedReplay && (ForQueryReplayTick == (FakeClientTick+4)), true);

	DiffSqrtError = ClientActorPrediction
		.GetRigidBodyState(ForQueryReplayTick)
		.CalculatedSumDiffSqrtError(GeneratedState[5]);

	TestEqual(TEXT("Check replay state at FakeClientTick+4"), DiffSqrtError, 0.0f);
#pragma endregion

#pragma region Shift by -4
	ClientActorPrediction.ShiftBufferElementsToDifferentClientTick(-4);

	NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

	TestEqual(TEXT("Shifting replay to FakeClientTick"), NeedReplay && (ForQueryReplayTick == FakeClientTick), true);

	DiffSqrtError = ClientActorPrediction
		.GetRigidBodyState(ForQueryReplayTick)
		.CalculatedSumDiffSqrtError(GeneratedState[5]);
	
	TestEqual(TEXT("Check replay state at Shifting-to FakeClientTick"), DiffSqrtError, 0.0f);
#pragma endregion

#pragma region Shift by 4
	ClientActorPrediction.ShiftBufferElementsToDifferentClientTick(4);

	NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

	TestEqual(TEXT("Shifting replay back to FakeClientTick+4"), NeedReplay && (ForQueryReplayTick == (FakeClientTick+4)), true);

	DiffSqrtError = ClientActorPrediction
		.GetRigidBodyState(ForQueryReplayTick)
		.CalculatedSumDiffSqrtError(GeneratedState[5]);

	TestEqual(TEXT("Check replay state at Shifting-back FakeClientTick+4"), DiffSqrtError, 0.0f);
#pragma endregion

#pragma region FakeClientTick - 2U

	ClientActorPrediction.ServerCorrectState
	(
		GeneratedState[5].GetRigidBodyState(),
		FakeClientTick - 2U
	);

	NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

	TestEqual(TEXT("Need replay at FakeClientTick-2"), NeedReplay && (ForQueryReplayTick == (FakeClientTick-2)), true);

	DiffSqrtError = ClientActorPrediction
		.GetRigidBodyState(ForQueryReplayTick)
		.CalculatedSumDiffSqrtError(GeneratedState[5]);

	TestEqual(TEXT("Check replay state at FakeClientTick - 2U"), DiffSqrtError, 0.0f);

#pragma endregion

#pragma region FakeClientTick + 12U
	ClientActorPrediction.ServerCorrectState
	(
		GeneratedState[5].GetRigidBodyState(),
		FakeClientTick + 12U
	);

	NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

	TestEqual(TEXT("Need replay at FakeClientTick+12"), NeedReplay && (ForQueryReplayTick == (FakeClientTick + 12)), true);

	DiffSqrtError = ClientActorPrediction
		.GetRigidBodyState(ForQueryReplayTick, false)
		.CalculatedSumDiffSqrtError(GeneratedState[5]);

	TestEqual(TEXT("Check correct state at FakeClientTick+12"), DiffSqrtError, 0.0f);
#pragma endregion

	

#pragma region ConsumeReplayFlag
	ClientActorPrediction.ConsumeReplayFlag();

	NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

	TestEqual(TEXT("Consume Replay Flag"), NeedReplay, false);
#pragma endregion

	return true;
}
