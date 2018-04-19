// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * Implement CircularBuffer by wrapping over TArray.
 * Current implementation doesn't call removed element's destructor.
 * Element can be removed by adding/inserting overflow element or removing.
 * Use MemoryOps.h DestructItem later.
 */
template<typename ElementType, typename AllocatorType=FDefaultAllocator>
class NETPHYSSYNC_API TNPSCircularBuffer
{
public:
	TNPSCircularBuffer(int32 MaxCapacityParam)
		: MaxCapacity(MaxCapacityParam)
		, CurrentStartIndex(0)
		, ElementCount(0)
		, Elements()
	{
		checkf(MaxCapacity > 0U, TEXT("Circular Buffer with Zero Capacity?"));
		Elements.AddDefaulted(MaxCapacity);
	}

	~TNPSCircularBuffer()
	{

	}

	FORCEINLINE ElementType& operator[](int32 Index)
	{
		int32 CalculateIndex = 0;
		CalculateArrayIndex(Index, CalculateIndex);
		return Elements[CalculateIndex];
	}

	FORCEINLINE const ElementType& operator[](int32 Index) const
	{
		int32 CalculateIndex = 0;
		CalculateArrayIndex(Index, CalculateIndex);
		return Elements[CalculateIndex];
	}

	void Add(ElementType&& Item)
	{
		if (Num() < Capacity())
		{
			++ElementCount;
			(*this)[ElementCount - 1] = Item;
		}
		else
		{
			(*this)[0] = Item;
			++CurrentStartIndex;

			if (CurrentStartIndex >= Capacity())
			{
				CurrentStartIndex -= Capacity();
			}
		}
	}

	void Add(const ElementType& Item)
	{
		if (Num() < Capacity())
		{
			++ElementCount;
			(*this)[ElementCount - 1] = Item;
		}
		else
		{
			(*this)[0] = Item;
			++CurrentStartIndex;

			if (CurrentStartIndex >= Capacity())
			{
				CurrentStartIndex -= Capacity();
			}
		}
	}

	/**
	 * Default element is created using default constructor.
	 */
	void AddDefaulted(uint32 Count=1)
	{
		ElementType DefaultElement = ElementType();
		for (uint32 i = 0; i < Count; ++i)
		{
			Add(DefaultElement);
		}
	}

	void Empty(bool SetUnOccupliedSlotToDefault=true)
	{
		RemoveAt(0, Capacity(), SetUnOccupliedSlotToDefault);
	}

	void RemoveAt(int32 Index, int32 Count=1, bool SetUnOccupliedSlotToDefault=true)
	{
		if (Count <= 0)
		{
			return;
		}

		checkf(IsIndexInRange(Index), TEXT("Index Argument is out of range."));
		if (Index + Count >= ElementCount) // If we only remove tail, Just update element count instead.
		{
			ElementCount = Index;
		}
		else if (Index == 0) // If we only remove the head, Just update index instead.
		{
			ElementCount -= Count;
			CurrentStartIndex += Count;
			if (CurrentStartIndex > Capacity())
			{
				CurrentStartIndex -= Capacity();
			}
		}
		else if(Num() - Index - Count < Index) // Choose between relocating direction.
		{
			// Relocate from tail to head.
			int32 ReplaceIndex = Index;
			int32 ToMoveIndex = Index + Count;

			for (int32 i = 0; i < Num() - ToMoveIndex; ++i)
			{
				(*this)[ReplaceIndex + i] = (*this)[ToMoveIndex + i];
			}

			ElementCount -= Count;
			checkf(ElementCount > 0, TEXT("Why is element count not positive?"));
		}
		else
		{
			// Relocate from head to tail.
			int32 ReplaceIndex = Index + Count - 1;
			int32 ToMoveIndex = Index - 1;

			for (int32 i = 0; i < Index; ++i)
			{
				(*this)[ReplaceIndex - i] = (*this)[ToMoveIndex - i];
			}

			CurrentStartIndex += Count;

			if (CurrentStartIndex > Capacity())
			{
				CurrentStartIndex -= Capacity();
			}

			ElementCount -= Count;
			checkf(ElementCount > 0, TEXT("Why is element count not positive?"));
		}

		if (SetUnOccupliedSlotToDefault)
		{
			ElementType DefaultValue = ElementType();
			for (int32 i = 0; i < Capacity(); ++i)
			{
				bool IsInRange =
					(
						// Not overflow case
						i >= CurrentStartIndex &&
						i - CurrentStartIndex < ElementCount
					)
					||
					(
						// Overflow case
						i < CurrentStartIndex &&
						Capacity() - CurrentStartIndex + i < ElementCount
					);

				if (!IsInRange)
				{
					Elements[i] = DefaultValue;
				}
			}
		}
	}

	/**
	 * Default element is created using default constructor.
	 * The shifted overflow tail element is discarded.
	 */
	void InsertDefaulted(int32 Index, int32 Count=1)
	{
		if (Count <= 0)
		{
			return;
		}

		checkf(IsIndexInRange(Index), TEXT("Index is out of range."));
		ElementType DefaultElement = ElementType();
		if (Index == 0) // Insert in front of head.
		{
			ElementCount += Count;

			if (ElementCount > Capacity())
			{
				ElementCount = Capacity();
			}

			if (CurrentStartIndex >= Count) // Handle uint32 overflow
			{
				CurrentStartIndex -= Count;
			}
			else if(Count < Capacity())
			{
				CurrentStartIndex = Capacity() + CurrentStartIndex - Count;
			}

			for (int32 i = 0; i < Count && i < ElementCount; ++i)
			{
				(*this)[i] = DefaultElement;
			}
		}
		// Choose between relocating direction.
		else if ( ElementCount >= Index + Count && // This condition handle overflow.
				Index < ElementCount - Index - Count)
		{
			// Relocate from tail to head.
			ElementCount += Count;

			if (ElementCount > Capacity())
			{
				ElementCount = Capacity();
			}

			// Handle overflow case
			if (Index + 1 + Count >= Capacity())
			{
				for (int32 i = Index + 1; i < ElementCount; ++i)
				{
					(*this)[i] = DefaultElement;
				}
			}
			else
			{
				// Shift CurrentStartIndex to left.
				if (CurrentStartIndex >= Count)
				{
					CurrentStartIndex -= Count;
				}
				else
				{
					CurrentStartIndex = Capacity() + CurrentStartIndex - Count;
				}

				// Move old element first
				for (int32 i = 0; i < Index; ++i)
				{
					(*this)[i] = (*this)[i + Count];
				}

				// Insert Default
				for (int32 i = Index; i < Index + Count && i < ElementCount; ++i)
				{
					(*this)[i] = DefaultElement;
				}

				checkf(CurrentStartIndex >= 0 && CurrentStartIndex < Capacity(), 
					TEXT("Wrong CurrentStartIndex Calculation."));
			}

		}
		else
		{
			// Relocate from head to tail.
			ElementCount = Num() + Count;
			
			if (ElementCount > Capacity())
			{
				ElementCount = Capacity();				
			}

			int32 ToReplaceIndex = ElementCount - 1;

			int32 ToMoveIndex = 0;
			
			if (ElementCount >= Count + 1) // Handle overflow.
			{
				ToMoveIndex = ElementCount - 1 - Count;
			}

			int32 NumMoveElement = 0;
			
			if (ToMoveIndex+1 > Index) // Handle overflow.
			{
				NumMoveElement = ToMoveIndex - Index + 1;
			}

			for (int32 i = 0; i < NumMoveElement; ++i)
			{
				(*this)[ToReplaceIndex - i] = (*this)[ToMoveIndex - i];
			}

			for (int32 i = Index; i < Index+Count && i < ElementCount; ++i)
			{
				(*this)[i] = DefaultElement;
			}
		}

	}

	FORCEINLINE int32 Num() const
	{
		return ElementCount;
	}

	FORCEINLINE int32 Capacity() const
	{
		return MaxCapacity;
	}

	FORCEINLINE bool IsIndexInRange(int32 Index) const
	{
		return Index >= 0 && Index < Num();
	}

	FORCEINLINE bool IsFull() const
	{
		return Num() == Capacity();
	}

private:
	TArray<ElementType, AllocatorType> Elements;
	int32 CurrentStartIndex;
	int32 ElementCount;
	int32 MaxCapacity;

	FORCEINLINE void CalculateArrayIndex(int32 Index, int32& OutCalculatedIndex) const
	{
		checkf(IsIndexInRange(Index), TEXT("Index %u is out of bound from size %u"), Index, Num());
		OutCalculatedIndex = CurrentStartIndex + Index;

		if (OutCalculatedIndex >= Capacity())
		{
			OutCalculatedIndex -= Capacity();
		}
	}
};
