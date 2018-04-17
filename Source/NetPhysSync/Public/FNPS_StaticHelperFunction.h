// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

template <class InterfaceType> class TScriptInterface;
class INetPhysSync;

/**
 * 
 */
class NETPHYSSYNC_API FNPS_StaticHelperFunction
{
public:
	/**
	 * This handles tick overflow.
	 */
	static void CalculateBufferArrayIndex(uint32 BufferStartTickIndex, uint32 BufferTargetIndex, int32& ResultArrayIndex);


	static void UnregisterINetPhySync(TScriptInterface<INetPhysSync> ToUnregister);

	/**
	 * If OverrideBufferMaxSize is zero, will use current TArray<ElementType, AllocatorType>::Max() as value.
	 * if new element is add to TArray, OverrideBufferMaxSize is used to clamp array.
	 */
	template<typename ElementType, typename AllocatorType>
	static void SetBuffers(TArray<ElementType, AllocatorType>& Buffers,
		const ElementType& ToSet, uint32& InOutBufferStartIndex,
		uint32 BufferSetTargetIndex,
		uint32 OverrideBufferMaxSize = 0)
	{
		if (Buffers.Num() == 0)
		{
			int32 OutArrayIndex;
			FNPS_StaticHelperFunction::CalculateBufferArrayIndex(InOutBufferStartIndex, BufferSetTargetIndex, OutArrayIndex);

			if (OutArrayIndex >= 0 && OutArrayIndex < Buffers.Num())
			{
				Buffers[OutArrayIndex] = ToSet;
			}
			else if (OutArrayIndex >= Buffers.Num())
			{
				checkf(OutArrayIndex == Buffers.Num(), TEXT("Why set array index very far in future?"));

				int32 AddAmount = OutArrayIndex - Buffers.Num() + 1;

				if (OverrideBufferMaxSize == 0)
				{
					OverrideBufferMaxSize = Buffers.Max();
				}

				int32 OverflowAmount = Buffers.Num() + AddAmount - OverrideBufferMaxSize;
				if (OverflowAmount > 0)
				{
					Buffers.RemoveAt(0, OverflowAmount, false);
					InOutBufferStartIndex += OverflowAmount;
				}

				Buffers.AddDefaulted(OutArrayIndex - Buffers.Num() + 1);
				Buffers[OutArrayIndex] = ToSet;
			}
			else if (OutArrayIndex < 0)
			{
				InOutBufferStartIndex += OutArrayIndex;
				Buffers.InsertDefaulted(0, -OutArrayIndex);
				Buffers[OutArrayIndex] = ToSet;
			}
		}
		else
		{
			InOutBufferStartIndex = InOutBufferStartIndex;
			Buffers.Add(ToSet);
		}
	}
};
