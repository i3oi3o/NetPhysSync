// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "TNPSCircularBuffer.h"

template <class InterfaceType> class TScriptInterface;
class INetPhysSync;
class UNPSNetSetting;
class UWorld;

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

	static int32 CalculateBufferArrayIndex(uint32 BufferStartTickIndex, uint32 BufferTargetIndex);

	/**
	 * This prevent warping overflow uint32 for cache tick.
	 * Tick is always update. So, when warping happen, it can potentially cause bug.
	 */
	static bool IsCachTickTooOld(uint32 CachTick, uint32 CurrentTick);

	/**
	 * Use this to determined if cache old tick is too old
	 * to prevent warping over overflow uint32.
	 */
	static int32 GetPositiveInclusiveThresholdForOldTick();

	static void UnregisterINetPhySync(TScriptInterface<INetPhysSync> ToUnregister);

	static UNPSNetSetting* GetNetSetting();

	static uint32 GetCurrentPhysTickIndex(const UObject* Obj);

	template<typename ElementType, typename AllocatorType>
	static void SetElementToBuffers
	(
		TNPSCircularBuffer<ElementType, AllocatorType>& Buffers,
		const ElementType& ToSet, 
		uint32& InOutBufferStartIndex,
		uint32 BufferSetTargetIndex
	)
	{
		if (Buffers.Num() > 0)
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
				if (-OutArrayIndex >= Buffers.Capacity())
				{
					InOutBufferStartIndex = BufferSetTargetIndex;
					Buffers.Empty();
					Buffers.Add(ToSet);
				}
				else
				{
					InOutBufferStartIndex += OutArrayIndex;
					Buffers.InsertDefaulted(0, -OutArrayIndex);
					Buffers[0] = ToSet;
				}
				
			}
		}
		else
		{
			InOutBufferStartIndex = BufferSetTargetIndex;
			Buffers.Add(ToSet);
		}
	}
};
