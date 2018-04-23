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
	void SaveInput(const FSavedInput& ToSave, uint32 ClientTickIndex);
	const FSavedInput& GetSavedInput(uint32 ClientTick, bool UseNearestIfOutOfBound=true) const;
	FORCEINLINE bool HasUnacknowledgedInput() const;
	/**
	 * If current don't have any unacknowledged input, 
	 * this return TickIndex for comparing with server replication's last acknowledged input.
	 */
	uint32 GetOldestUnacknowledgeInputClientTickIndex() const;
	virtual void ShiftBufferElementsToDifferentClientTick(int32 ShiftAmount) override;
	virtual void ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex);

	template<typename ArrayAllocator>
	void CopyUnacknowledgeInputToArray
	(
		TArray<FSavedInput, ArrayAllocator>& DestArray,
		uint32& CopyUnacknowledgedStartTickIndex
	) const
	{
		DestArray.RemoveAt(0, DestArray.Num(), false);
		if (HasUnacknowledgedInput())
		{
			int32 OutArrayIndex;
			
			FNPS_StaticHelperFunction::CalculateBufferArrayIndex
			(
				ClientInputBuffersStartTickIndex,
				GetOldestUnacknowledgeInputClientTickIndex(),
				OutArrayIndex
			);

			if (OutArrayIndex < 0)
			{
				OutArrayIndex = 0;
				CopyUnacknowledgedStartTickIndex = ClientInputBuffersStartTickIndex;
			}
			else
			{
				CopyUnacknowledgedStartTickIndex = GetOldestUnacknowledgeInputClientTickIndex();
			}


			if (OutArrayIndex >= 0 && OutArrayIndex < ClientInputBuffers.Num())
			{
				int32 Amount = ClientInputBuffers.Num() - OutArrayIndex;
				DestArray.SetNumUninitialized(Amount, false);
				
				for (int32 i = OutArrayIndex; i < ClientInputBuffers.Num(); ++i)
				{
					DestArray[i-OutArrayIndex] = ClientInputBuffers[i];
				}
			}
		}
	}


protected:
	TNPSCircularBuffer<FSavedInput, TInlineAllocator<NPS_BUFFER_SIZE>> ClientInputBuffers;
	uint32 ClientInputBuffersStartTickIndex;
	uint32 OldestUnacknowledgedInput;
};
