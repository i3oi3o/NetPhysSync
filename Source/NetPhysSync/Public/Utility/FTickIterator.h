// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

enum class ETickIteratorEndRange : uint8
{
	INCLUSIVE_END,
	EXCLUSIVE_END
};

enum class ETickIteratorDir : int8
{
	NONE=0,
	FORWARD=1,
	BACKWARD=-1
};

/**
 * 
 */
struct NETPHYSSYNC_API FTickIterator
{
public:
	FTickIterator(uint32 BeginTickParam, uint32 EndTickParam, ETickIteratorEndRange EndRange);
	~FTickIterator();

	FORCEINLINE FTickIterator& operator++()
	{
		++CurrentIdx;
		return *this;
	}
	
	FORCEINLINE FTickIterator& operator+=(int32 Offset)
	{
		CurrentIdx += Offset;
		return *this;
	}

	FORCEINLINE uint32 operator*() const
	{
		return BeginTick + Dir*CurrentIdx;
	}

	FORCEINLINE explicit operator bool() const
	{
		return CurrentIdx >= 0 && CurrentIdx <= IdxInclusiveRange;
	}

	FORCEINLINE ETickIteratorDir GetDir() const
	{
		return static_cast<ETickIteratorDir>(Dir);
	}

private:
	uint32 BeginTick;
	uint32 EndTick;
	int32 IdxInclusiveRange;
	int32 CurrentIdx;
	int8 Dir;
};
