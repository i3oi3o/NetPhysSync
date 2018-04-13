// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.
#include "AutomationTest.h"
#include "NumericLimits.h"
#include "NPS_StaticHelperFunction.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUint32OverflowTest, "NetPhysSync.OverflowHandle.Uint32 Uint32OperationTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBufferTickOverflow, "NetPhysSync.OverflowHandle.BufferIndex BufferIndexOperationTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)

bool FBufferTickOverflow::RunTest(const FString& Parameters)
{
	bool TestResult = true;
	
	int32 TestIndex;
	uint32 HalfUint32 = TNumericLimits<uint32>::Max() / 2;

	NPS_StaticHelperFunction::CalculateBufferArrayIndex(TNumericLimits<uint32>::Max(), 0U, TestIndex);
	TestResult &= (TestIndex == 1);
	
	NPS_StaticHelperFunction::CalculateBufferArrayIndex(TNumericLimits<uint32>::Max()-2U, 3U, TestIndex);
	TestResult &= (TestIndex == 6);

	NPS_StaticHelperFunction::CalculateBufferArrayIndex(HalfUint32-5U, HalfUint32+5U, TestIndex);
	TestResult &= (TestIndex == 10);

	NPS_StaticHelperFunction::CalculateBufferArrayIndex(HalfUint32, HalfUint32 + 5U, TestIndex);
	TestResult &= (TestIndex == 5);

	NPS_StaticHelperFunction::CalculateBufferArrayIndex(0U, TNumericLimits<uint32>::Max(), TestIndex);
	TestResult &= (TestIndex == -1);

	NPS_StaticHelperFunction::CalculateBufferArrayIndex(3U, TNumericLimits<uint32>::Max()-2U, TestIndex);
	TestResult &= (TestIndex == -6);

	NPS_StaticHelperFunction::CalculateBufferArrayIndex(HalfUint32 + 5U, HalfUint32 - 5U, TestIndex);
	TestResult &= (TestIndex == -10);

	NPS_StaticHelperFunction::CalculateBufferArrayIndex(HalfUint32 + 5U, HalfUint32, TestIndex);
	TestResult &= (TestIndex == -5);

	return TestResult;
}