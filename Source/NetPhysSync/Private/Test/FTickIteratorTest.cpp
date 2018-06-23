// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "AutomationTest.h"
#include "FTickIterator.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTickIteratorTest, "NetPhysSync.Utility.TickIterator", EAutomationTestFlags::EditorContext | EAutomationTestFlags::SmokeFilter)
bool FTickIteratorTest::RunTest(const FString& Param)
{
	TArray<uint32> VerifyTickArray({ 0-4U, 0-3U, 0-2U, 0-1U, 0, 1U, 2U, 3U, 4U });
	uint32 BeginTick = VerifyTickArray[0];
	uint32 EndTick = VerifyTickArray[VerifyTickArray.Num()-1];

	{
		FTickIterator EmptyIterator(BeginTick, BeginTick, 
			ETickIteratorEndRange::EXCLUSIVE_END);

		if (EmptyIterator.GetDir() != ETickIteratorDir::NONE)
		{
			AddError(TEXT("Empty Iterator's direction is wrong."));
			return false;
		}

		if (EmptyIterator)
		{
			AddError(TEXT("Empty Iterator doesn't work."));
			return false;
		}
	}


	{
		FTickIterator ForwardEndInclusive(BeginTick, EndTick, 
			ETickIteratorEndRange::INCLUSIVE_END);

		if (ForwardEndInclusive.GetDir() != ETickIteratorDir::FORWARD)
		{
			AddError(TEXT("Forward iterator's direction is wrong."));
			return false;
		}

		int32 i = 0;
		for (;ForwardEndInclusive; ++ForwardEndInclusive)
		{
			if (!VerifyTickArray.IsValidIndex(i) || 
				VerifyTickArray[i] != *ForwardEndInclusive)
			{
				AddError(TEXT("Forward end inclusive iterator doesn't work."));
				return false;
			}
			++i;
		}
	}

	{
		FTickIterator BackwardEndInclusive(EndTick, BeginTick, 
			ETickIteratorEndRange::INCLUSIVE_END);

		if (BackwardEndInclusive.GetDir() != ETickIteratorDir::BACKWARD)
		{
			AddError(TEXT("Backward iterator's direction is wrong."));
			return false;
		}

		int32 i = VerifyTickArray.Num() - 1;
		for (; BackwardEndInclusive; ++BackwardEndInclusive)
		{
			if (!VerifyTickArray.IsValidIndex(i) ||
				VerifyTickArray[i] != *BackwardEndInclusive)
			{
				AddError(TEXT("Backward end inclusive iterator doesn't work."));
				return false;
			}
			--i;
		}
	}

	{
		FTickIterator ForwardEndExclusive(BeginTick, EndTick + 1, 
			ETickIteratorEndRange::EXCLUSIVE_END);

		if (ForwardEndExclusive.GetDir() != ETickIteratorDir::FORWARD)
		{
			AddError(TEXT("Forward iterator's direction is wrong."));
			return false;
		}

		int32 i = 0;
		for (; ForwardEndExclusive; ++ForwardEndExclusive)
		{
			if (!VerifyTickArray.IsValidIndex(i) ||
				VerifyTickArray[i] != *ForwardEndExclusive)
			{
				AddError(TEXT("Forward end exclusive iterator doesn't work."));
				return false;
			}
			++i;
		}
	}

	{
		FTickIterator BackwardEndExclusive(EndTick, BeginTick - 1, 
			ETickIteratorEndRange::EXCLUSIVE_END);
		
		if (BackwardEndExclusive.GetDir() != ETickIteratorDir::BACKWARD)
		{
			AddError(TEXT("Backward iterator's direction is wrong."));
			return false;
		}

		int32 i = VerifyTickArray.Num() - 1;
		for (; BackwardEndExclusive; ++BackwardEndExclusive)
		{
			if (!VerifyTickArray.IsValidIndex(i) ||
				VerifyTickArray[i] != *BackwardEndExclusive)
			{
				AddError(TEXT("Backward end exclusive iterator doesn't work."));
				return false;
			}
			--i;
		}
	}

	return true;
}