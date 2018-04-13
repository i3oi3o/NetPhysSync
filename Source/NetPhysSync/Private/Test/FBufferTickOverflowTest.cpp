// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.
#include "AutomationTest.h"
#include "NumericLimits.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUint32OverflowTest, "NetPhysSync.OverflowHandle.Uint32 Uint32OperationTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)

bool FUint32OverflowTest::RunTest(const FString& Parameters)
{
	bool testResult = true;
	uint32 x = 2;
	uint32 y = 4;
	uint32 z = 7;

	testResult &= (5 == ((x - y)+ z));

	testResult &= ((TNumericLimits<uint32>::Max() - x+1U) == (x - y));

	testResult &= (x - (TNumericLimits<uint32>::Max() - x + 1U) == x+x);

	testResult &= (TNumericLimits<uint32>::Max() +1U == 0U);

	return testResult;
}

