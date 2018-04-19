// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FNPS_ClientPredictionBase.h"
#include "FSavedClientRigidBodyState.h"
#include "TNPSCircularBuffer.h"

namespace physx
{
	class PxRigidDynamic;
}

/**
 * 
 */
class NETPHYSSYNC_API FNPS_ClientActorPrediction : public FNPS_ClientPredictionBase
{
public:
	FNPS_ClientActorPrediction();
	virtual ~FNPS_ClientActorPrediction();

	void SaveRigidBodyState(physx::PxRigidDynamic* PxRigidBodyDynamic, uint32 ClientTickIndex);
	/*
	* Currently for testing.
	*/
	void SaveRigidBodyState(const FSavedClientRigidBodyState& SaveRigidBodyState, uint32 ClientTickIndex);
	void GetRigidBodyState(physx::PxRigidDynamic* PxRigidDynamic, uint32 ClientTickIndex, bool bUseNearestIfTickOutOfRange=true) const;
	/**
	 * Currently for testing.
	 */
	FSavedClientRigidBodyState GetRigidBodyState(uint32 ClientTickIndex, bool bUseNearestIfTickOutOfRange = true) const;
	virtual void ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex);
	virtual void ShiftStartBufferIndex(int32 ShiftAmount);
	bool TryGetReplayTickIndex(uint32& OutTickIndex) const;
	void ConsumeReplayFlag();
	bool HasClientStateBufferYet() const;

protected:
	/*
	* The size here is hard code. Current physic tick is 20 ms.
	* For 200 ms round trip time, We need array of 10 slot.
	* Use 20 slot to avoid overflow.
	*/
	TNPSCircularBuffer<FSavedClientRigidBodyState, TInlineAllocator<20>> ClientStateBuffers;
	uint32 ClientStateBufferStartsTickIndex;
	uint32 LastCorrectedStateTickIndex;
	bool bNeedReplay;
};


