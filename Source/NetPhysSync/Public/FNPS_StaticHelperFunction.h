// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "TNPSCircularBuffer.h"

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


	template<typename ElementType, typename AllocatorType>
	static void SetElementToBuffers
	(
		TNPSCircularBuffer<ElementType, AllocatorType>& Buffers,
		const ElementType& ToSet, 
		uint32& InOutBufferStartIndex,
		uint32 BufferSetTargetIndex
	)
	{
		if (Buffers.Num() == 0)
		{
			int32 OutArrayIndex;
			FNPS_StaticHelperFunction::CalculateBufferArrayIndex
			(InOutBufferStartIndex, BufferSetTargetIndex, OutArrayIndex);

			if (Buffers.IsIndexInRange(OutArrayIndex))
			{
				Buffers[OutArrayIndex] = ToSet;
			}
			else if (OutArrayIndex >= Buffers.Num())
			{
				ensureMsgf(OutArrayIndex == Buffers.Num(), TEXT("This shouldn't happen. FNetPhysSyncManager should handle this."));

				int32 AddAmount = OutArrayIndex - Buffers.Num() + 1;

				if (AddAmount >= Buffers.Capacity())
				{
					Buffers.Empty();
					InOutBufferStartIndex = BufferSetTargetIndex;
				}
				else
				{
					if (Buffers.Num() + AddAmount > Buffers.Capacity())
					{
						InOutBufferStartIndex += Buffers.Num() + AddAmount - Buffers.Capacity();
					}

					if (AddAmount > 1)
					{
						Buffers.AddDefaulted(AddAmount - 1);
					}
				}

				
				Buffers.Add(ToSet);
			}
			else if (OutArrayIndex < 0)
			{
				InOutBufferStartIndex += OutArrayIndex;
				Buffers.InsertDefaulted(0, -OutArrayIndex);
				Buffers[0] = ToSet;
			}
		}
		else
		{
			InOutBufferStartIndex = BufferSetTargetIndex;
			Buffers.Add(ToSet);
		}
	}
};
