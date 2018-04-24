// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.
#include "AutomationTest.h"
#include "NumericLimits.h"
#include "FNPS_StaticHelperFunction.h"
#include "FNPS_ClientActorPrediction.h"
#include "FSavedClientRigidBodyState.h"
#include "FSavedInput.h"
#include "FNPS_ClientPawnPrediction.h"



void GenerateFakedStateFunction(FSavedClientRigidBodyState* InSaveClientStateArray, int32 Count)
{
	for (int32 i = 0; i < Count; ++i)
	{
		new (InSaveClientStateArray+i) FReplicatedRigidBodyState
		(
			FVector(static_cast<float>(i + 1), 0.0f, 0.0f),
			FQuat(FRotator(0.0f, static_cast<float>(i + 1), 0.0f)),
			FVector(0.0f, static_cast<float>(i + 1), 0.0f),
			FVector(0.0f, 0.0f, static_cast<float>(i + 1)),
			false
		);
	}
}

void GenerateFakedInputFunction(FSavedInput* InSaveInputArray, int32 Count)
{
	for (int32 i = 0; i < Count; ++i)
	{
		new (InSaveInputArray + i) FSavedInput
		(
			FVector(static_cast<float>(i+1), 0.0f, 0.0f)
		);
	}
}

template<typename AllocatorType>
void TestCopyUnacknowledgedInput
(
	const FString& PrefixDesc,
	const FNPS_ClientPawnPrediction& ClientPawnPrediction,
	TArray<FSavedInput, AllocatorType>& ForStoreUnacknowledgedInputArray,
	uint32 VerifyCopyUnacknowledgeStartTick,
	FAutomationTestBase* CurrentAutomationTest
)
{
	uint32 CopyUnacknowledgedStartTick;

	ClientPawnPrediction
		.CopyUnacknowledgedInputToArray
		(
			ForStoreUnacknowledgedInputArray, 
			CopyUnacknowledgedStartTick
		);

	CurrentAutomationTest->TestEqual
	(
		PrefixDesc+TEXT("Test unacknowledge client tick."),
		CopyUnacknowledgedStartTick,
		VerifyCopyUnacknowledgeStartTick
	);

	int32 CountLastEmpty = 0;
	bool bCountingLastEmpty = true;
	for (int32 i = ForStoreUnacknowledgedInputArray.Num() - 1; i >= 0; --i)
	{
		bool bIsEqual = ForStoreUnacknowledgedInputArray[i] ==
			ClientPawnPrediction.GetSavedInput(CopyUnacknowledgedStartTick + i);

		CurrentAutomationTest->TestEqual
		(
			PrefixDesc+TEXT("Test unacknowledge input value."),
			bIsEqual,
			true
		);

		if (bCountingLastEmpty)
		{
			if (ForStoreUnacknowledgedInputArray[i].IsEmptyInput())
			{
				++CountLastEmpty;
			}
			else
			{
				bCountingLastEmpty = false;
			}
		}
	}

	CurrentAutomationTest->TestEqual
	(
		PrefixDesc+TEXT("Last empty input count from buffer shouldn't exceed 1."),
		CountLastEmpty <= 1,
		true
	);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientActorPredictionHasBufferYetTest, "NetPhysSync.PredictBuffer.Client.Actor.HasStateBuffer", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
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

	FReplicatedRigidBodyState DummyReplicatedState
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientActorPredictionSaveAndGetTest, "NetPhysSync.PredictBuffer.Client.Actor.SaveAndGet", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientActorPredictionSaveAndGetTest::RunTest(const FString& Parameters)
{
	
	uint32 FakeClientTick = 0 - 5U;
	const int32 OverflowSize = 10;
	FNPS_ClientActorPrediction ClientActorPrediction;
	FSavedClientRigidBodyState GenerateClientRigidBodyStates[OverflowSize];

	GenerateFakedStateFunction(GenerateClientRigidBodyStates, OverflowSize);

	FReplicatedRigidBodyState (DummyReplicatedState)
	(
		FVector(1.0f, 0.0f, 0.0f),
		FQuat(EForceInit::ForceInit),
		FVector(),
		FVector(),
		true
	);

	FSavedClientRigidBodyState DummyState(DummyReplicatedState);



	for (int32 i = 0; i < NPS_BUFFER_SIZE; ++i)
	{
		ClientActorPrediction.SaveRigidBodyState(DummyState, FakeClientTick+i);
	}

	for (int32 i = 0; i < NPS_BUFFER_SIZE; ++i)
	{
		float ErrorDiff = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick+i)
			.CalculatedSumDiffSqrtError(DummyReplicatedState);

		TestEqual(TEXT("Compare Dummy RigidBodyState in buffer."), ErrorDiff, 0.0f);
	}

	for (int32 i = 0; i < OverflowSize; ++i)
	{
		ClientActorPrediction.SaveRigidBodyState
		(
			GenerateClientRigidBodyStates[i], 
			FakeClientTick + NPS_BUFFER_SIZE +i
		);
	}

	uint32 ToVerifyClientTick = FakeClientTick + NPS_BUFFER_SIZE - OverflowSize;

	for (int32 i = 0; i < NPS_BUFFER_SIZE; ++i)
	{
		float ErrorDiff = 0.0f;
		if (i < OverflowSize)
		{
			ErrorDiff = ClientActorPrediction
				.GetRigidBodyState(ToVerifyClientTick + i)
				.CalculatedSumDiffSqrtError(DummyReplicatedState);
			TestEqual(TEXT("Compare Dummy2 RigidBodyState in buffer."), ErrorDiff, 0.0f);
		}
		else
		{
			ErrorDiff = ClientActorPrediction
				.GetRigidBodyState(ToVerifyClientTick + i)
				.CalculatedSumDiffSqrtError(GenerateClientRigidBodyStates[i- OverflowSize]);
			TestEqual(TEXT("Compare Generated RigidBodyState in buffer."), ErrorDiff, 0.0f);
		}
	}

	{
		/*Test Query Out of bound element*/
		int32 ShiftAmountToOutOfBound = 2 * (NPS_BUFFER_SIZE + OverflowSize);

		float ErrorDiff = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick + ShiftAmountToOutOfBound)
			.CalculatedSumDiffSqrtError(GenerateClientRigidBodyStates[OverflowSize-1]);

		TestEqual(TEXT("Comapre out of bound state from future using nearest."), ErrorDiff, 0.0f);

		ErrorDiff = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick - ShiftAmountToOutOfBound)
			.CalculatedSumDiffSqrtError(DummyReplicatedState);

		TestEqual(TEXT("Compare out of bound state from past using nearest."), ErrorDiff, 0.0f);


		bool bIsValid = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick + ShiftAmountToOutOfBound, false)
			.IsReplicatedStateValid();

		TestEqual(TEXT("Test out of bound state from future without using nearest."), bIsValid, false);

		bIsValid = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick - ShiftAmountToOutOfBound, false)
			.IsReplicatedStateValid();

		TestEqual(TEXT("Test out of bound state from past without using nearest."), bIsValid, false);
	}

	return true;

}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientActorPredictionShiftBufferTest, "NetPhysSync.PredictBuffer.Client.Actor.ShiftBufferGetAndSave", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientActorPredictionShiftBufferTest::RunTest(const FString& Parameters)
{
	uint32 FakeClientTick = 0U - 2U;

	FNPS_ClientActorPrediction ClientActorPrediction;

	const int32 OverflowSize = 10;

	FSavedClientRigidBodyState GeneratedSaveState[NPS_BUFFER_SIZE + OverflowSize];

	GenerateFakedStateFunction(GeneratedSaveState, NPS_BUFFER_SIZE + OverflowSize);

	for (int32 i = 0; i < NPS_BUFFER_SIZE + OverflowSize; ++i)
	{
		ClientActorPrediction.SaveRigidBodyState(GeneratedSaveState[i], FakeClientTick+i);
	}

	ClientActorPrediction.ShiftElementsToDifferentTickIndex(-5);

	

	for (int32 i = 0; i < NPS_BUFFER_SIZE; ++i)
	{
		float DiffError = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick + i + OverflowSize - 5)
			.CalculatedSumDiffSqrtError(GeneratedSaveState[i+OverflowSize]);

		TestEqual(TEXT("Compare Shift-to-Past Buffer Value"), DiffError, 0.0f);
	}

	ClientActorPrediction.ShiftElementsToDifferentTickIndex(10);


	for (int32 i = 0; i < NPS_BUFFER_SIZE; ++i)
	{
		float DiffError = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick + i + OverflowSize + 5)
			.CalculatedSumDiffSqrtError(GeneratedSaveState[i+OverflowSize]);

		TestEqual(TEXT("Compare Shift-to-Future Buffer Value"), DiffError, 0.0f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientActorPredictionReplayTickTest, "NetPhysSync.PredictBuffer.Client.Actor.GetReplayTickTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientActorPredictionReplayTickTest::RunTest(const FString& Parameters)
{
	uint32 FakeClientTick = 0U - 2U;
	FNPS_ClientActorPrediction ClientActorPrediction;
	const int32 TestSize = static_cast<int32>(0.5f*NPS_BUFFER_SIZE) + 1;
	
	checkf(TestSize > 1, TEXT("Test Size is too small."));

	FSavedClientRigidBodyState GeneratedState[TestSize];
	uint32 ForQueryReplayTick;
	float DiffSqrtError = 0;

	GenerateFakedStateFunction(GeneratedState, TestSize);

	for (int32 i = 0; i < TestSize; ++i)
	{
		ClientActorPrediction.SaveRigidBodyState
		(
			GeneratedState[i], 
			FakeClientTick+i
		);
	}


#pragma region Correct Same State
	{
		int32 TestIndex = static_cast<int32>(0.2f*TestSize);

		ClientActorPrediction.ServerCorrectState
		(
			GeneratedState[TestIndex].GetReplicatedRigidBodyState(),
			FakeClientTick + TestIndex
		);

		bool NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

		TestEqual(TEXT("Correct State is the same. Shouldn't need replay."), NeedReplay, false);

	}
#pragma endregion

#pragma region Correct Slightly Different
	{
		int32 TestIndex = static_cast<int32>(0.2f*TestSize);
		const FReplicatedRigidBodyState& ExistState = ClientActorPrediction
			.GetRigidBodyState(FakeClientTick + TestIndex)
			.GetReplicatedRigidBodyState();

		FReplicatedRigidBodyState SlightlyDifferent
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
			FakeClientTick + TestIndex
		);

		bool NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

		TestEqual(TEXT("Correct State is slightly different. Shouldn't need replay."), NeedReplay, false);
	}
#pragma endregion

#pragma region FakeClientTick + TestIndex
	{
		int32 TestIndex = static_cast<int32>(0.2f*TestSize);

		ClientActorPrediction.ServerCorrectState
		(
			GeneratedState[TestSize-1].GetReplicatedRigidBodyState(),
			FakeClientTick + TestIndex
		);

		bool NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

		TestEqual
		(
			TEXT("Need replay at FakeClientTick + TestIndex"), 
			NeedReplay && (ForQueryReplayTick == (FakeClientTick + TestIndex)), 
			true
		);

		DiffSqrtError = ClientActorPrediction
			.GetRigidBodyState(ForQueryReplayTick)
			.CalculatedSumDiffSqrtError(GeneratedState[TestSize - 1]);
		
		TestEqual(TEXT("Check replay state at FakeClientTick+TestIndex"), DiffSqrtError, 0.0f);
	}
#pragma endregion

#pragma region Shift by -TestIndex
	{
		int32 TestIndex = static_cast<int32>(0.2f*TestSize);

		ClientActorPrediction.ShiftElementsToDifferentTickIndex(-TestIndex);

		bool NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

		TestEqual(TEXT("Shifting replay to FakeClientTick"), NeedReplay && (ForQueryReplayTick == FakeClientTick), true);

		DiffSqrtError = ClientActorPrediction
			.GetRigidBodyState(ForQueryReplayTick)
			.CalculatedSumDiffSqrtError(GeneratedState[TestSize - 1]);

		TestEqual(TEXT("Check replay state at Shifting-to FakeClientTick"), DiffSqrtError, 0.0f);
	}
#pragma endregion

#pragma region Shift by + TestIndex
	{
		int32 TestIndex = static_cast<int32>(0.2f*TestSize);

		ClientActorPrediction.ShiftElementsToDifferentTickIndex(TestIndex);

		bool NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

		TestEqual(TEXT("Shifting replay back to FakeClientTick + TestIndex"), NeedReplay && (ForQueryReplayTick == (FakeClientTick + TestIndex)), true);

		DiffSqrtError = ClientActorPrediction
			.GetRigidBodyState(ForQueryReplayTick)
			.CalculatedSumDiffSqrtError(GeneratedState[TestSize - 1]);

		TestEqual(TEXT("Check replay state at Shifting-back FakeClientTick + TestIndex"), DiffSqrtError, 0.0f);
	}
#pragma endregion

#pragma region FakeClientTick - 2U
	{
		ClientActorPrediction.ServerCorrectState
		(
			GeneratedState[TestSize - 1].GetReplicatedRigidBodyState(),
			FakeClientTick - 2
		);

		bool NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

		TestEqual(TEXT("Need replay at FakeClientTick - 2"), NeedReplay && (ForQueryReplayTick == (FakeClientTick - 2)), true);

		DiffSqrtError = ClientActorPrediction
			.GetRigidBodyState(ForQueryReplayTick)
			.CalculatedSumDiffSqrtError(GeneratedState[TestSize - 1]);

		TestEqual(TEXT("Check replay state at FakeClientTick - 2"), DiffSqrtError, 0.0f);
	}
#pragma endregion

#pragma region FakeClientTick + TestSize + 2U
	{
		ClientActorPrediction.ServerCorrectState
		(
			GeneratedState[TestSize - 1].GetReplicatedRigidBodyState(),
			FakeClientTick + TestSize + 2U
		);

		bool NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

		TestEqual
		(
			TEXT("Need replay at FakeClientTick + TestSize + 2U"), 
			NeedReplay && (ForQueryReplayTick == (FakeClientTick + TestSize + 2U)), 
			true
		);

		DiffSqrtError = ClientActorPrediction
			.GetRigidBodyState(ForQueryReplayTick, false)
			.CalculatedSumDiffSqrtError(GeneratedState[TestSize - 1]);

		TestEqual(TEXT("Check correct state at FakeClientTick + TestSize + 2U"), DiffSqrtError, 0.0f);
	}
#pragma endregion

	

#pragma region ConsumeReplayFlag
	{
		ClientActorPrediction.ConsumeReplayFlag();

		bool NeedReplay = ClientActorPrediction.TryGetReplayTickIndex(ForQueryReplayTick);

		TestEqual(TEXT("Consume Replay Flag"), NeedReplay, false);
	}
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientPawnPredictionSaveAndGetTest, "NetPhysSync.PredictBuffer.Client.Pawn.GetAndSaveInput", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientPawnPredictionSaveAndGetTest::RunTest(const FString& Parameters)
{
	uint32 FakeClientTick = 0 - 10U;
	FNPS_ClientPawnPrediction PawnPrediction;

	const int32 OverflowSize = 10;
	const int32 TestSize = NPS_BUFFER_SIZE + OverflowSize;
	FSavedInput GeneratedInput[TestSize];

	GenerateFakedInputFunction(GeneratedInput, NPS_BUFFER_SIZE-1);

	checkf(NPS_BUFFER_SIZE > 1, TEXT("Buffer size is too small."));

	for (int32 i = NPS_BUFFER_SIZE-1; i < TestSize; ++i)
	{
		new (GeneratedInput + i)FSavedInput();
	}

	for (int32 i = 0; i < TestSize; ++i)
	{
		PawnPrediction.SaveInput(GeneratedInput[i], FakeClientTick + i);
	}

	for (int32 i = 0; i < NPS_BUFFER_SIZE+1; ++i)
	{
		bool bIsEqual = PawnPrediction.GetSavedInput(FakeClientTick + i) == GeneratedInput[i];
		TestEqual(TEXT("Test Save and Get Value"), bIsEqual, true);
	}

	for (int32 i = -10; i < 0; ++i)
	{
		bool bIsEqual = PawnPrediction.GetSavedInput(FakeClientTick + i) == GeneratedInput[0];

		TestEqual(TEXT("Test Nearest from past"), bIsEqual, true);

		const FSavedInput& EmptyInput = PawnPrediction
			.GetSavedInput(FakeClientTick + i, false);

		TestEqual
		(
			TEXT("Test out of bound from past. Should get empty input."),
			EmptyInput.IsEmptyInput(),
			true
		);
	}


	for (int32 i = TestSize; i < TestSize+10; ++i)
	{
		bool bIsEqual = PawnPrediction.GetSavedInput(FakeClientTick + i) == GeneratedInput[TestSize-1];

		TestEqual(TEXT("Test Nearest from future"), bIsEqual, true);

		const FSavedInput& EmptyInput = PawnPrediction
			.GetSavedInput(FakeClientTick + i, false);

		TestEqual
		(
			TEXT("Test out of bound from future. Should get empty input."),
			EmptyInput.IsEmptyInput(),
			true
		);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientPawnPredictionShiftTest, "NetPhysSync.PredictBuffer.Client.Pawn.ShiftBufferGetAndSave", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientPawnPredictionShiftTest::RunTest(const FString& Parameters)
{
	uint32 FakeClientTick = 0 - 10U;
	FNPS_ClientPawnPrediction ClientPawnPrediction;

	const int32 TestSize = static_cast<int32>(0.5f*NPS_BUFFER_SIZE);

	FSavedInput GeneratedInput[TestSize];

	GenerateFakedInputFunction(GeneratedInput, TestSize);

	for (int32 i = 0; i < TestSize; ++i)
	{
		ClientPawnPrediction.SaveInput(GeneratedInput[i], FakeClientTick + i);
	}

	ClientPawnPrediction.ShiftElementsToDifferentTickIndex(-5);

	for (int32 i = 0; i < TestSize; ++i)
	{
		const FSavedInput& SavedInput = ClientPawnPrediction
			.GetSavedInput(FakeClientTick - 5 + i);

		TestEqual
		(
			TEXT("Test Shift To Past."), 
			SavedInput == GeneratedInput[i], 
			true
		);
	}

	ClientPawnPrediction.ShiftElementsToDifferentTickIndex(10);

	for (int32 i = 0; i < TestSize; ++i)
	{
		const FSavedInput& SavedInput = ClientPawnPrediction
			.GetSavedInput(FakeClientTick + 5 + i);

		TestEqual
		(
			TEXT("Test Shift To Past."),
			SavedInput == GeneratedInput[i],
			true
		);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClientPawnPredictionUnacknowledgedInputTest, "NetPhysSync.PredictBuffer.Client.Pawn.UnacknowledgeInput", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FClientPawnPredictionUnacknowledgedInputTest::RunTest(const FString& Parameters)
{
	uint32 FakeClientTick = 0 - 5U;

	FNPS_ClientPawnPrediction ClientPawnPredicton;

	const int32 TestSize = static_cast<int32>(0.5f*NPS_BUFFER_SIZE);
	const int32 AdditionSize = static_cast<int32>(0.5f*TestSize);

	checkf(TestSize > 1, TEXT("TestSize is too small"));

	TArray<FSavedInput, TInlineAllocator<TestSize+1>> UnacknowledgedInputArray;

	FSavedInput GeneratedSaveInput[TestSize];

	GenerateFakedInputFunction(GeneratedSaveInput, TestSize);

	for (int32 i = 0; i < TestSize; ++i)
	{
		ClientPawnPredicton.SaveInput(GeneratedSaveInput[i], FakeClientTick + i);
	}

	

	for (int32 i = 0; i < AdditionSize; ++i)
	{
		ClientPawnPredicton.SaveInput(FSavedInput(), FakeClientTick + TestSize + i);
	}

	TestEqual
	(
		TEXT("Has Unacknowledged Input."), 
		ClientPawnPredicton.HasUnacknowledgedInput(), 
		true
	);

	ClientPawnPredicton.ShiftElementsToDifferentTickIndex(-5);

	TestEqual
	(
		TEXT("Test shift-to-past unacknowledged input index."),
		ClientPawnPredicton.GetOldestUnacknowledgeInputClientTickIndex(),
		FakeClientTick - 5
	);

	ClientPawnPredicton.ShiftElementsToDifferentTickIndex(10);

	TestEqual
	(
		TEXT("Test shift-to-future unacknowledged input index."),
		ClientPawnPredicton.GetOldestUnacknowledgeInputClientTickIndex(),
		FakeClientTick + 5
	);


	ClientPawnPredicton.ShiftElementsToDifferentTickIndex(-5);

	
	TestCopyUnacknowledgedInput
	(
		TEXT("Test after shift back to old tick : "),
		ClientPawnPredicton, 
		UnacknowledgedInputArray, 
		FakeClientTick,
		this
	);

	FReplicatedRigidBodyState DummyState
	(
		FVector(1.0f, 0.0f, 0.0f),
		FQuat(EForceInit::ForceInit),
		FVector(0.0f, 1.0f, 0.0f),
		FVector(0.0f, 0.0f, 0.0f),
		false
	);

	ClientPawnPredicton.ServerCorrectState(DummyState, FakeClientTick+AdditionSize);

	TestEqual
	(
		TEXT("Test if there is unacknowledged input after correcting state."),
		ClientPawnPredicton.HasUnacknowledgedInput(),
		true
	);

	TestCopyUnacknowledgedInput
	(
		TEXT("Test after correct state : "),
		ClientPawnPredicton, 
		UnacknowledgedInputArray, 
		FakeClientTick + AdditionSize,
		this
	);

	ClientPawnPredicton.ServerCorrectState(DummyState, FakeClientTick + TestSize+1);

	TestEqual
	(
		TEXT("Test if there is unacknowledged input after correcting all input."),
		ClientPawnPredicton.HasUnacknowledgedInput(),
		false
	);

	for (int32 i = 0; i < AdditionSize; ++i)
	{
		ClientPawnPredicton.SaveInput(GeneratedSaveInput[i], FakeClientTick + TestSize + AdditionSize + i);
	}

	TestCopyUnacknowledgedInput
	(
		TEXT("Test new input after acknowledged all : "),
		ClientPawnPredicton, 
		UnacknowledgedInputArray, 
		FakeClientTick + TestSize + AdditionSize,
		this
	);

	return true;
}