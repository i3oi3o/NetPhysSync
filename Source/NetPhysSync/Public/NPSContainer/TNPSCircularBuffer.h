// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * Implement CircularBuffer by wrapping over TArray
 */
template<typename ElementType, typename AllocatorType=FDefaultAllocator>
class NETPHYSSYNC_API TNPSCircularBuffer
{
public:
	TNPSCircularBuffer(uint32 MaxCapacityParam)
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

	FORCEINLINE ElementType& operator[](uint32 Index)
	{
		uint32 CalculateIndex = 0;
		CalculateArrayIndex(Index, CalculateIndex);
		return Elements[CalculateIndex];
	}

	FORCEINLINE const ElementType& operator[](uint32 Index) const
	{
		uint32 CalculateIndex = 0;
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

	void RemoveAt(uint32 Index, uint32 Count=1, bool SetUnOccupliedSlotToDefault=true)
	{
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
			uint32 ReplaceIndex = Index;
			uint32 ToMoveIndex = Index + Count;

			for (uint32 i = 0; i < Num() - ToMoveIndex; ++i)
			{
				(*this)[ReplaceIndex + i] = (*this)[ToMoveIndex + i];
			}

			ElementCount -= Count;
			checkf(ElementCount > 0, TEXT("Why is element count not positive?"));
		}
		else
		{
			// Relocate from head to tail.
			uint32 ReplaceIndex = Index + Count - 1;
			uint32 ToMoveIndex = Index - 1;

			for (uint32 i = 0; i < Index; ++i)
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
			for (uint32 i = 0; i < Capacity(); ++i)
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
	void InsertDefault(uint32 Index, uint32 Count=1)
	{
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

			for (uint32 i = 0; i < Count && i < ElementCount; ++i)
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
				for (uint32 i = Index + 1; i < ElementCount; ++i)
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
				for (uint32 i = 0; i < Index; ++i)
				{
					(*this)[i] = (*this)[i + Count];
				}

				// Insert Default
				for (uint32 i = Index; i < Index + Count && i < ElementCount; ++i)
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

			uint32 ToReplaceIndex = ElementCount - 1;

			uint32 ToMoveIndex = 0;
			
			if (ElementCount >= Count + 1) // Handle overflow.
			{
				ToMoveIndex = ElementCount - 1 - Count;
			}

			uint32 NumMoveElement = 0;
			
			if (ToMoveIndex+1 > Index) // Handle overflow.
			{
				NumMoveElement = ToMoveIndex - Index + 1;
			}

			for (uint32 i = 0; i < NumMoveElement; ++i)
			{
				(*this)[ToReplaceIndex - i] = (*this)[ToMoveIndex - i];
			}

			for (uint32 i = Index; i < Index+Count && i < ElementCount; ++i)
			{
				(*this)[i] = DefaultElement;
			}
		}

	}

	FORCEINLINE uint32 Num() const
	{
		return ElementCount;
	}

	FORCEINLINE uint32 Capacity() const
	{
		return MaxCapacity;
	}

	FORCEINLINE bool IsIndexInRange(uint32 Index)
	{
		return Index >= 0 && Index < Num();
	}

private:
	TArray<ElementType, AllocatorType> Elements;
	uint32 CurrentStartIndex;
	uint32 ElementCount;
	uint32 MaxCapacity;

	FORCEINLINE void CalculateArrayIndex(uint32 Index, uint32& OutCalculatedIndex)
	{
		checkf(IsIndexInRange(Index), TEXT("Index %u is out of bound from size %u"), Index, Num());
		OutCalculatedIndex = CurrentStartIndex + Index;

		if (OutCalculatedIndex >= Capacity())
		{
			OutCalculatedIndex -= Capacity();
		}
	}
};
