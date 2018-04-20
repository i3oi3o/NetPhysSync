// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.
#include "AutomationTest.h"
#include "NPSContainer/TNPSCircularBuffer.h"


typedef TNPSCircularBuffer<int32, TInlineAllocator<10>> TestCircularBuffer;



IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCircularBufferAddingTest, "NetPhysSync.CircularBuffer.Add", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FCircularBufferAddingTest::RunTest(const FString& Parameters)
{
	TestCircularBuffer Test(10);

	for (int32 i = 0; i < 10; ++i)
	{
		Test.Add(i);
		if (Test[i] != i)
		{
			return false;
		}
	}

	for (int32 i = 10; i < 15; ++i)
	{
		Test.Add(i);
	}

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (Test[i] != i+5)
		{
			return false;
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCircularBufferAddingDefaultTest, "NetPhysSync.CircularBuffer.AddDefault", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FCircularBufferAddingDefaultTest::RunTest(const FString& Parameters)
{
	TestCircularBuffer Test(10);

	for (int32 i = 0; i < 8U; ++i)
	{
		Test.Add(i);
	}

	Test.AddDefaulted(3);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i >= 7)
		{
			TestEqual(TEXT("Buffer Value"), Test[i], int32());
		}
		else
		{
			TestEqual(TEXT("Buffer Value"), Test[i], i+1);
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCircularBufferRemovingTest, "NetPhysSync.CircularBuffer.Remove", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FCircularBufferRemovingTest::RunTest(const FString& Parameters)
{
	TestCircularBuffer Test(10);

	for (int32 i = 0; i < 10U; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(0, 5);
		
	TestEqual(TEXT("Remove Head-Buffer Size:"), Test.Num(), 5);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Remove Head-Buffer Value:"), Test[i], i+5);
	}

	Test.RemoveAt(0, 5);

	TestEqual(TEXT("Remove All-Buffer Size:"), Test.Num(), 0);

	for (int32 i = 0; i < 10U; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(5, 5);

	TestEqual(TEXT("Remove Tail-Buffer Size:"), Test.Num(), 5);
	
	for (int32 i = 0; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Remove Tail-Buffer Value:"), Test[i], i);
	}

	Test.RemoveAt(0, 5);

	TestEqual(TEXT("Remove All-Buffer Size:"), Test.Num(), 0);

	for (int32 i = 0; i < 10; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(1, 2);

	TestEqual(TEXT("Remove middle-near-head-Buffer Size:"), Test.Num(), 8);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i == 0)
		{
			TestEqual(TEXT("Remove middle-near-head-Buffer Value:"), Test[i], 0);
		}
		else
		{
			TestEqual(TEXT("Remove middle-near-head-Buffer Value:"),Test[i], i+2);
		}
	}

	Test.RemoveAt(0, 10);

	TestEqual(TEXT("Remove all-Buffer Size:"), Test.Num(), 0);

	for (int32 i = 0; i < 10; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(7, 2);

	TestEqual(TEXT("Remove middle-near-tail-Buffer Size:"), Test.Num(), 8);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i == Test.Num() - 1)
		{
			TestEqual(TEXT("Remove middle-near-tail-Buffer Value:"), Test[i], 9);
		}
		else
		{
			TestEqual(TEXT("Remove middle-near-tail-Buffer Value:"), Test[i], i);
		}
	}

	Test.Empty();

	TestEqual(TEXT("Empty Buffer Size:"), Test.Num(), 0);

	for (int32 i = 0; i < 10; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(1, 8);

	TestEqual(TEXT("Remove Middle-BufferSize:"), Test.Num(), 2);
	TestEqual(TEXT("Remove Middle-BufferValue:"), Test[0], 0);
	TestEqual(TEXT("Remove Middle-BufferValue:"), Test[1], 9);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCircularBufferInsertTest, "NetPhysSync.CircularBuffer.Insert", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FCircularBufferInsertTest::RunTest(const FString& Parameters)
{
	TestCircularBuffer Test(10);

	for (int32 i = 1U; i <= 8; ++i)
	{
		Test.Add(i);
	}

	Test.InsertDefaulted(0, 2);

	TestEqual(TEXT("Insert Default to Head Buffer Size"), Test.Num(), 10);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 2U)
		{
			TestEqual(TEXT("Insert default to head buffer, value:"), Test[i], int32());
		}
		else
		{
			TestEqual(TEXT("Insert default to head buffer, value:"), Test[i], i - 1);
		}
	}

	Test.InsertDefaulted(0, 2);

	TestEqual(TEXT("Insert default to head overflow buffer, size:"), Test.Num(), 10);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 4)
		{
			TestEqual(TEXT("Insert default to head buffer, value:"), Test[i], int32());
		}
		else
		{
			TestEqual(TEXT("Insert default To head buffer value:"), Test[i], i - 3);
		}
	}

	Test.Empty();

	for (int32 i = 1; i <= 5; ++i)
	{
		Test.Add(i);
	}

	Test.InsertDefaulted(0, 11);

	TestEqual(TEXT("Insert default to head overflow buffer, size:"), Test.Num(), 10);
	for (int32 i = 0; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Insert default to head overflow, size:"), Test[i], int32());	
	}

	Test.Empty();

	for (int32 i = 1; i <= 10; ++i)
	{
		Test.Add(i);
	}

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Current Buffer Value"), Test[i], i+1);
	}

	Test.InsertDefaulted(0U, 10U);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Current Buffer Value"), Test[i], int32());
	}

	Test.Empty();

	for (int32 i = 1; i <= 10U; ++i)
	{
		Test.Add(i);
	}

	Test.InsertDefaulted(1, 11);

	TestEqual(TEXT("Insert default to near-head buffer till overflow, size:"), Test.Num(), 10);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i == 0U)
		{
			TestEqual(TEXT("Insert default to near-head buffer, overflow size, value:"), Test[i], 1);
		}
		else
		{
			TestEqual(TEXT("Insert default to near-head buffer, overflow size, value:"), Test[i], uint32());
		}
	}


	Test.Empty();

	for (int32 i = 1; i <= 5; ++i)
	{
		Test.Add(i);
	}
	Test.InsertDefaulted(1, 2);

	TestEqual(TEXT("Insert default to near-head buffer, size:"), Test.Num(), 7);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 1)
		{
			TestEqual(TEXT("Insert default to near-head buffer, value:"), Test[i], 1);
		}
		else if ( i < 3)
		{
			TestEqual(TEXT("Insert default to near-head buffer, value:"), Test[i], int32());
		}
		else
		{
			TestEqual(TEXT("Insert default to near-head buffer, value:"), Test[i], i-1);
		}
	}

	Test.Empty();

	for (int32 i = 1U; i <= 5; ++i)
	{
		Test.Add(i);
	}
	Test.InsertDefaulted(1, 5);

	TestEqual(TEXT("Insert default to near-head buffer, size:"), Test.Num(), 10);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 1)
		{
			TestEqual(TEXT("Insert default to near-head buffer2, value:"), Test[i], 1);
		}
		else if (i < 6)
		{
			TestEqual(TEXT("Insert default to near-head buffer2, value:"), Test[i], int32());
		}
		else
		{
			TestEqual(TEXT("Insert default to near-head buffer2, value:"), Test[i], i - 4);
		}
	}


	Test.Empty();

	for (int32 i = 1; i <= 10; ++i)
	{
		Test.Add(i);
	}
	Test.InsertDefaulted(2, 5);

	TestEqual(TEXT("Insert default to near-head buffer3, size:"), Test.Num(), 10);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 2)
		{
			TestEqual(TEXT("Insert default to near-head buffer3, value:"), Test[i], i+1);
		}
		else if (i < 7)
		{
			TestEqual(TEXT("Insert default to near-head buffer3, value:"), Test[i], int32());
		}
		else
		{
			TestEqual(TEXT("Insert default to near-head buffer3, value:"), Test[i], i - 4);
		}
	}


	Test.Empty();

	for (int32 i = 1; i <= 8; ++i)
	{
		Test.Add(i);
	}

	Test.InsertDefaulted(6, 2);

	TestEqual(TEXT("Insert default to near-tail buffer, size:"), Test.Num(), 10);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 6)
		{
			TestEqual(TEXT("Insert default to near-tail buffer, value:"), Test[i], i+1);
		}
		else if (i < 8)
		{
			TestEqual(TEXT("Insert default to near-tail buffer, value:"), Test[i], int32());
		}
		else
		{
			TestEqual(TEXT("Insert default to near-tail buffer, value:"), Test[i], i - 1);
		}
	}

	Test.InsertDefaulted(8, 11);

	TestEqual(TEXT("Insert default to near-tail buffer till overflow, size:"), Test.Num(), 10);

	for (int32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 6)
		{
			TestEqual(TEXT("Insert default to near-tail buffer till overflow, value:"), Test[i], i+1);
		}
		else
		{
			TestEqual(TEXT("Insert default to near-tail buffer till overflow, value:"), Test[i], int32());
		}
	}

	return true;
}