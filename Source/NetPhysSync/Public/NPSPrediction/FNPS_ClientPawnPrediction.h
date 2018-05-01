// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FNPS_ClientActorPrediction.h"
#include "FNPS_StaticHelperFunction.h"
#include "TNPSCircularBuffer.h"
#include "FSavedInput.h"


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
	const FSavedInput& GetSavedInput(uint32 ClientTick, bool bUseNearestIfOutOfBound=true) const;
	FORCEINLINE bool HasUnacknowledgedInput() const;
	/**
	 * If current don't have any unacknowledged input, 
	 * this return TickIndex for comparing with server replication's last acknowledged input.
	 */
	bool TryGetOldestUnacknowledgeInputTickIndex(uint32& OutTickIndex) const;
	virtual void ShiftElementsToDifferentTickIndex(int32 ShiftAmount) override;
	virtual void ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex);
	FORCEINLINE FBufferInfo GetInputBufferInfo() const;
	
	/**
	 * Should be called every tick.
	 */
	virtual void Update(uint32 CurrentTickIndex) override;

	template<typename ArrayAllocator>
	void CopyUnacknowledgedInputToArray
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
				ClientInputBufferStartTickIndex,
				OldestUnacknowledgedInputTick,
				OutArrayIndex
			);

			if (OutArrayIndex < 0)
			{
				OutArrayIndex = 0;
				CopyUnacknowledgedStartTickIndex = ClientInputBufferStartTickIndex;
			}
			else
			{
				CopyUnacknowledgedStartTickIndex = OldestUnacknowledgedInputTick;
			}


			if (OutArrayIndex >= 0 && OutArrayIndex < ClientInputBuffer.Num())
			{
				int32 Amount = ClientInputBuffer.Num() - OutArrayIndex;
				DestArray.SetNumUninitialized(Amount, false);
				
				for (int32 i = OutArrayIndex; i < ClientInputBuffer.Num(); ++i)
				{
					DestArray[i-OutArrayIndex] = ClientInputBuffer[i];
				}
			}
		}
	}


protected:
	TNPSCircularBuffer<FSavedInput, TInlineAllocator<NPS_BUFFER_SIZE>> ClientInputBuffer;
	uint32 ClientInputBufferStartTickIndex;
	uint32 OldestUnacknowledgedInputTick;
	bool bIsOldestUnacknowledgeInputTooOld;
};
