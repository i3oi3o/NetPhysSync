// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FNPS_ClientActorPrediction.h"
#include "NetPrediction/FSavedInput.h"
#include "TNPSCircularBuffer.h"

/**
 * 
 */
class NETPHYSSYNC_API FNPS_ClientPawnPrediction : public FNPS_ClientActorPrediction
{
	typedef FNPS_ClientActorPrediction Super;

public:
	FNPS_ClientPawnPrediction();
	virtual ~FNPS_ClientPawnPrediction();

	void SaveInput(FVector TargetWorldSpeed, uint32 ClientTickIndex);
	FVector GetSavedInput(uint32 ClientTick) const;
	bool HasUnacknowledgedInput() const;
	FORCEINLINE uint32 GetUnacknowledgeInputClientTickIndex() const;
	virtual void ShiftStartBufferIndex(int32 ShiftAmount) override;
	bool HasClientInputBuffers() const;

	template<typename ArrayAllocator>
	void CopyUnacknowledgeInputToArray(TArray<FSavedInput, ArrayAllocator> DestArray)
	{
		if (HasUnacknowledgedInput())
		{

		}
	}


protected:
	/**
	 * The inline allocator's size is hard code.
	 * The number is based of 20 ms physic tick duration and 200 ms round trip time.
	 * This give us buffer length with 10 slots. To avoid overflow making it 20 slots.
	 */
	TNPSCircularBuffer<FSavedInput, TInlineAllocator<20>> ClientInputBuffers;
	uint32 ClientInputBuffersStartTickIndex;
};
