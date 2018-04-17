// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.
#include "AutomationTest.h"
#include "NPSContainer/TNPSCircularBuffer.h"


typedef TNPSCircularBuffer<uint32, TInlineAllocator<10>> TestCircularBuffer;



IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCircularBufferAddingTest, "NetPhysSync.CircularBuffer.Add", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FCircularBufferAddingTest::RunTest(const FString& Parameters)
{
	TestCircularBuffer Test(10);

	for (uint32 i = 0; i < 10; ++i)
	{
		Test.Add(i);
		if (Test[i] != i)
		{
			return false;
		}
	}

	for (uint32 i = 10; i < 15; ++i)
	{
		Test.Add(i);
	}

	for (uint32 i = 0; i < Test.Num(); ++i)
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

	for (uint32 i = 0; i < 8U; ++i)
	{
		Test.Add(i);
	}

	Test.AddDefaulted(3);

	UE_LOG(LogTemp, Log, TEXT("uint32 default constructor:%u"), uint32());

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i >= 7)
		{
			TestEqual(TEXT("Buffer Value"), Test[i], uint32());
		}
		else
		{
			TestEqual(TEXT("Buffer Value"), Test[i], i+1U);
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCircularBufferRemovingTest, "NetPhysSync.CircularBuffer.Remove", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FCircularBufferRemovingTest::RunTest(const FString& Parameters)
{
	TestCircularBuffer Test(10);

	for (uint32 i = 0; i < 10U; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(0, 5);
		
	TestEqual(TEXT("Remove Head-Buffer Size:"), Test.Num(), 5U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Remove Head-Buffer Value:"), Test[i], i+5U);
	}

	Test.RemoveAt(0, 5);

	TestEqual(TEXT("Remove All-Buffer Size:"), Test.Num(), 0U);

	for (uint32 i = 0; i < 10U; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(5, 5);

	TestEqual(TEXT("Remove Tail-Buffer Size:"), Test.Num(), 5U);
	
	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Remove Tail-Buffer Value:"), Test[i], i);
	}

	Test.RemoveAt(0, 5);

	TestEqual(TEXT("Remove All-Buffer Size:"), Test.Num(), 0U);

	for (uint32 i = 0; i < 10U; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(1, 2);

	TestEqual(TEXT("Remove middle-near-head-Buffer Size:"), Test.Num(), 8U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i == 0)
		{
			TestEqual(TEXT("Remove middle-near-head-Buffer Value:"), Test[i], 0U);
		}
		else
		{
			TestEqual(TEXT("Remove middle-near-head-Buffer Value:"),Test[i], i+2U);
		}
	}

	Test.RemoveAt(0, 10);

	TestEqual(TEXT("Remove all-Buffer Size:"), Test.Num(), 0U);

	for (uint32 i = 0; i < 10; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(7, 2);

	TestEqual(TEXT("Remove middle-near-tail-Buffer Size:"), Test.Num(), 8U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i == Test.Num() - 1)
		{
			TestEqual(TEXT("Remove middle-near-tail-Buffer Value:"), Test[i], 9U);
		}
		else
		{
			TestEqual(TEXT("Remove middle-near-tail-Buffer Value:"), Test[i], i);
		}
	}

	Test.Empty();

	TestEqual(TEXT("Empty Buffer Size:"), Test.Num(), 0U);

	for (uint32 i = 0; i < 10U; ++i)
	{
		Test.Add(i);
	}

	Test.RemoveAt(1, 8);

	TestEqual(TEXT("Remove Middle-BufferSize:"), Test.Num(), 2U);
	TestEqual(TEXT("Remove Middle-BufferValue:"), Test[0], 0U);
	TestEqual(TEXT("Remove Middle-BufferValue:"), Test[1], 9U);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCircularBufferInsertTest, "NetPhysSync.CircularBuffer.Insert", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FCircularBufferInsertTest::RunTest(const FString& Parameters)
{
	TestCircularBuffer Test(10);

	for (uint32 i = 1U; i <= 8U; ++i)
	{
		Test.Add(i);
	}

	Test.InsertDefault(0, 2);

	TestEqual(TEXT("Insert Default to Head Buffer Size"), Test.Num(), 10U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 2U)
		{
			TestEqual(TEXT("Insert default to head buffer, value:"), Test[i], uint32());
		}
		else
		{
			TestEqual(TEXT("Insert default to head buffer, value:"), Test[i], i - 1U);
		}
	}

	Test.InsertDefault(0U, 2U);

	TestEqual(TEXT("Insert default to head overflow buffer, size:"), Test.Num(), 10U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 4U)
		{
			TestEqual(TEXT("Insert default to head buffer, value:"), Test[i], uint32());
		}
		else
		{
			TestEqual(TEXT("Insert default To head buffer value:"), Test[i], i - 3U);
		}
	}

	Test.Empty();

	for (uint32 i = 1U; i <= 5U; ++i)
	{
		Test.Add(i);
	}

	Test.InsertDefault(0U, 11U);

	TestEqual(TEXT("Insert default to head overflow buffer, size:"), Test.Num(), 10U);
	for (uint32 i = 0U; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Insert default to head overflow, size:"), Test[i], uint32());	
	}

	Test.Empty();

	for (uint32 i = 1; i <= 10U; ++i)
	{
		Test.Add(i);
	}

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Current Buffer Value"), Test[i], i+1U);
	}

	Test.InsertDefault(0U, 10U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		TestEqual(TEXT("Current Buffer Value"), Test[i], uint32());
	}

	Test.Empty();

	for (uint32 i = 1U; i <= 10U; ++i)
	{
		Test.Add(i);
	}

	Test.InsertDefault(1U, 11U);

	TestEqual(TEXT("Insert default to near-head buffer till overflow, size:"), Test.Num(), 10U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i == 0U)
		{
			TestEqual(TEXT("Insert default to near-head buffer, overflow size, value:"), Test[i], 1U);
		}
		else
		{
			TestEqual(TEXT("Insert default to near-head buffer, overflow size, value:"), Test[i], uint32());
		}
	}


	Test.Empty();

	for (uint32 i = 1U; i <= 5U; ++i)
	{
		Test.Add(i);
	}
	Test.InsertDefault(1U, 2U);

	TestEqual(TEXT("Insert default to near-head buffer, size:"), Test.Num(), 7U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 1U)
		{
			TestEqual(TEXT("Insert default to near-head buffer, value:"), Test[i], 1U);
		}
		else if ( i < 3U)
		{
			TestEqual(TEXT("Insert default to near-head buffer, value:"), Test[i], uint32());
		}
		else
		{
			TestEqual(TEXT("Insert default to near-head buffer, value:"), Test[i], i-1U);
		}
	}

	Test.Empty();

	for (uint32 i = 1U; i <= 5U; ++i)
	{
		Test.Add(i);
	}
	Test.InsertDefault(1U, 5U);

	TestEqual(TEXT("Insert default to near-head buffer, size:"), Test.Num(), 10U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 1U)
		{
			TestEqual(TEXT("Insert default to near-head buffer2, value:"), Test[i], 1U);
		}
		else if (i < 6U)
		{
			TestEqual(TEXT("Insert default to near-head buffer2, value:"), Test[i], uint32());
		}
		else
		{
			TestEqual(TEXT("Insert default to near-head buffer2, value:"), Test[i], i - 4U);
		}
	}


	Test.Empty();

	for (uint32 i = 1U; i <= 10U; ++i)
	{
		Test.Add(i);
	}
	Test.InsertDefault(2U, 4U);

	TestEqual(TEXT("Insert default to near-head buffer3, size:"), Test.Num(), 10U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 2U)
		{
			TestEqual(TEXT("Insert default to near-head buffer2, value:"), Test[i], i+1U);
		}
		else if (i < 6U)
		{
			TestEqual(TEXT("Insert default to near-head buffer2, value:"), Test[i], uint32());
		}
		else
		{
			TestEqual(TEXT("Insert default to near-head buffer2, value:"), Test[i], i - 3U);
		}
	}


	Test.Empty();

	for (uint32 i = 1U; i <= 8U; ++i)
	{
		Test.Add(i);
	}

	Test.InsertDefault(6U, 2U);

	TestEqual(TEXT("Insert default to near-tail buffer, size:"), Test.Num(), 10U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 6U)
		{
			TestEqual(TEXT("Insert default to near-tail buffer, value:"), Test[i], i+1U);
		}
		else if (i < 8U)
		{
			TestEqual(TEXT("Insert default to near-tail buffer, value:"), Test[i], uint32());
		}
		else
		{
			TestEqual(TEXT("Insert default to near-tail buffer, value:"), Test[i], i - 1U);
		}
	}

	Test.InsertDefault(8U, 11U);

	TestEqual(TEXT("Insert default to near-tail buffer till overflow, size:"), Test.Num(), 10U);

	for (uint32 i = 0; i < Test.Num(); ++i)
	{
		if (i < 6U)
		{
			TestEqual(TEXT("Insert default to near-tail buffer till overflow, value:"), Test[i], i+1U);
		}
		else
		{
			TestEqual(TEXT("Insert default to near-tail buffer till overflow, value:"), Test[i], uint32());
		}
	}

	return true;
}