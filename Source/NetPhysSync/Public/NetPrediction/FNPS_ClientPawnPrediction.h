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
	FORCEINLINE bool HasUnacknowledgedInput() const;
	/**
	 * This try to return last UnacknowledgeInput ClientTick index. 
	 * Regardless whether currently has UnackonwledgeInput or not. 
	 */
	uint32 GetLastUnacknowledgeInputClientTickIndex() const;
	virtual void ShiftStartBufferIndex(int32 ShiftAmount) override;
	virtual void ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex);

	template<typename ArrayAllocator>
	void CopyUnacknowledgeInputToArray(TArray<FSavedInput, ArrayAllocator>& DestArray)
	{
		DestArray.Empty(DestArray.Max());
		if (HasUnacknowledgedInput())
		{
			int32 OutArrayIndex;
			FNPS_StaticHelperFunction::CalculateBufferArrayIndex
			(
				ClientInputBuffersStartTickIndex,
				GetLastUnacknowledgeInputClientTickIndex(),
				OutArrayIndex
			);

			if (OutArrayIndex >= 0 && OutArrayIndex < ClientInputBuffers.Num())
			{
				int32 Amount = ClientInputBuffers.Num() - OutArrayIndex;

				if (DestArray.Max() < Amount)
				{
					DestArray.SetNum(Amount);
				}

				for (int32 i = OutArrayIndex; i < ClientInputBuffers.Num(); ++i)
				{
					DestArray.Add(ClientInputBuffers[i]);
				}
			}
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
	uint32 LastUnacknowledgeInput;
};
