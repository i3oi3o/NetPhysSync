// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FNPS_ClientActorPrediction.h"
#include "NetPrediction/FSavedInput.h"

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
	/**
	 * For copying to replication. Is there better way?
	 */
	const TArray<FSavedInput, TInlineAllocator<20>>& GetInputBuffers() const;
	/*
	* For copying to replication. Is there better way?
	*/
	uint32 GetInputClientStartTickIndex() const;
	virtual void ShiftStartBufferIndex(int32 ShiftAmount) override;

	bool HasClientInputBuffers() const;

protected:
	/**
	 * The inline allocator's size is hard code.
	 * The number is based of 20 ms physic tick duration and 200 ms round trip time.
	 * This give us buffer length with 10 slots. To avoid overflow making it 20 slots.
	 */
	TArray<FSavedInput, TInlineAllocator<20>> ClientInputBuffers;
	uint32 ClientInputBuffersStartTickIndex;
};
