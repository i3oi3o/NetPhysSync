// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FTickIterator.h"
#include "FNPS_StaticHelperFunction.h"

FTickIterator::FTickIterator
(
	uint32 BeginTickIdxParam, 
	uint32 EndTickIdxParam,
	ETickIteratorEndRange EndRange
)
	: BeginTick(BeginTickIdxParam)
	, EndTick(EndTickIdxParam)
{
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		BeginTick, EndTick, IdxInclusiveRange
	);

	CurrentIdx = 0;
	if (IdxInclusiveRange == 0)
	{
		Dir = 0;
	}
	else if (IdxInclusiveRange > 0)
	{
		Dir = 1;
	}
	else
	{
		Dir = -1;
		IdxInclusiveRange = -IdxInclusiveRange;
	}

	if (EndRange == ETickIteratorEndRange::EXCLUSIVE_END)
	{
		IdxInclusiveRange -= 1;
	}
	else if (EndRange != ETickIteratorEndRange::INCLUSIVE_END)
	{
		UE_LOG(LogTemp, Error, TEXT("Unknown ETickIteratorEndRange"));
	}
}

FTickIterator::~FTickIterator()
{
}
