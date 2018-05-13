// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FNPS_ClientPredictionBase.h"
#include "FSavedClientRigidBodyState.h"
#include "TNPSCircularBuffer.h"
#include "FBufferInfo.h"
#include "EIdxOutOfRangeHandle.h"

namespace physx
{
	class PxRigidDynamic;
}

/**
 * 
 */
class NETPHYSSYNC_API FNPS_ClientActorPrediction : public FNPS_ClientPredictionBase
{
	typedef FNPS_ClientPredictionBase Super;

public:
	FNPS_ClientActorPrediction();
	virtual ~FNPS_ClientActorPrediction();

	void SaveRigidBodyState(physx::PxRigidDynamic* PxRigidBodyDynamic, uint32 ClientTickIndex);
	/*
	* Currently for testing.
	*/
	void SaveRigidBodyState(const FSavedClientRigidBodyState& SaveRigidBodyState, uint32 ClientTickIndex);
	void GetRigidBodyState
	(
		physx::PxRigidDynamic* PxRigidDynamic, 
		uint32 ClientTickIndex, 
		EIdxOutOfRangeHandle eHandleOutOfBoundIndex=EIdxOutOfRangeHandle::UseNearestIndex
	) const;
	/**
	 * Currently for testing.
	 */
	const FSavedClientRigidBodyState& GetRigidBodyState
	(
		uint32 ClientTickIndex, 
		EIdxOutOfRangeHandle eHandleOutOfBoundIndex = EIdxOutOfRangeHandle::UseNearestIndex
	) const;
	virtual void ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex);
	virtual void ShiftElementsToDifferentTickIndex(int32 ShiftAmount);
	FORCEINLINE bool IsReplayTickIndex(uint32 TickIndex) const;
	FORCEINLINE bool TryGetReplayTickIndex(uint32& OutTickIndex) const;
	FORCEINLINE bool IsNeedReplay() const;
	FORCEINLINE bool TryGetLastCorrectStateTickIndex(uint32& OutTickIndex) const;
	FORCEINLINE bool HasClientStateBuffer() const;
	FORCEINLINE FBufferInfo GetStateBufferInfo() const;
	void ConsumeReplayFlag();




	/**
	 * Should be call every tick.
	 */
	virtual void Update(uint32 CurrentTickIndex);

protected:
	TNPSCircularBuffer<FSavedClientRigidBodyState, TInlineAllocator<NPS_BUFFER_SIZE>> ClientStateBuffer;
	uint32 ClientStateBufferStartTickIndex;
	uint32 LastCorrectedStateTickIndex;
	bool bNeedReplay;
	/**
	 * 
	 */
	bool bIsCorrectedStateIndexTooOld;
	

	template<typename ElementType, typename AllocatorType>
	void HandleOutOfBoundIndex
	(
		const TNPSCircularBuffer<ElementType, AllocatorType>& Buffer,
		EIdxOutOfRangeHandle eHandleMethod,
		int32& RefHandledIndex
	) const
	{
		bool bClamp = 
			eHandleMethod == EIdxOutOfRangeHandle::UseNearestIndex ||
			(
				RefHandledIndex >= Buffer.Num() &&
				eHandleMethod == EIdxOutOfRangeHandle::UseNearestIndexIfOutRangeFromEnd
			)
			||
			(
				RefHandledIndex < 0 &&
				eHandleMethod == EIdxOutOfRangeHandle::UseNearestIndexIfOutRangeFromBegin
			);
		
		if (bClamp)
		{
			Buffer.ClampIndexParamWithinRange(RefHandledIndex);
		}
#if !UE_BUILD_SHIPPING
		else 
		{
			ensureMsgf
			(
				eHandleMethod == EIdxOutOfRangeHandle::PreventExceptionByDoNothingOrReturnEmptyInvalid ||
				eHandleMethod == EIdxOutOfRangeHandle::UseNearestIndex ||
				eHandleMethod == EIdxOutOfRangeHandle::UseNearestIndexIfOutRangeFromBegin||
				eHandleMethod == EIdxOutOfRangeHandle::UseNearestIndexIfOutRangeFromEnd,
				TEXT("Doesn't implement EIdxOutOfRangeHandle::%u"),
				eHandleMethod
			);
		}
#endif
	}
};


