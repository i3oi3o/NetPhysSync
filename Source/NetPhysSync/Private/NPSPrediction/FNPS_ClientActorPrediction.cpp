// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ClientActorPrediction.h"
#include "FNPS_StaticHelperFunction.h"

FNPS_ClientActorPrediction::FNPS_ClientActorPrediction() 
	: Super(),
	ClientStateBuffer(20),
	ClientStateBufferStartTickIndex(0),
	LastCorrectedStateTickIndex(0),
	bNeedReplay(false),
	bIsCorrectedStateIndexTooOld(true)
{
}

FNPS_ClientActorPrediction::~FNPS_ClientActorPrediction()
{
}

void FNPS_ClientActorPrediction::SaveRigidBodyState(physx::PxRigidDynamic* PxRigidBodyDynamic, uint32 ClientTickIndex)
{
	this->SaveRigidBodyState(FSavedClientRigidBodyState(PxRigidBodyDynamic), ClientTickIndex);
}

void FNPS_ClientActorPrediction::SaveRigidBodyState(const FSavedClientRigidBodyState& SaveRigidBodyState, uint32 ClientTickIndex)
{
	FNPS_StaticHelperFunction::SetElementToBuffers
	(
		ClientStateBuffer,
		SaveRigidBodyState,
		ClientStateBufferStartTickIndex,
		ClientTickIndex
	);
}

void FNPS_ClientActorPrediction::GetRigidBodyState
(
	physx::PxRigidDynamic* PxRigidDynamic, 
	uint32 ClientTickIndex, 
	EIdxOutOfRangeHandle eHandleOutOfBoundIndex /*=UseNearestIndex*/
) const
{
	const FSavedClientRigidBodyState& RetrivedState = GetRigidBodyState(ClientTickIndex, eHandleOutOfBoundIndex);
	// Don't worry.
	// If state is invalid, FSavedClientRigidBodyState::RetriveBodyState() 
	// doesn't copy state to PxRigidDynamic.
	RetrivedState.GetReplicatedRigidBodyState(PxRigidDynamic);
}

const FSavedClientRigidBodyState& FNPS_ClientActorPrediction::GetRigidBodyState
(
	uint32 ClientTickIndex, 
	EIdxOutOfRangeHandle eHandleOutOfBoundIndex /*=UseNearestIndex*/
) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		ClientStateBufferStartTickIndex, ClientTickIndex,
		OutArrayIndex
	);

	HandleOutOfBoundIndex
	(
		ClientStateBuffer, 
		eHandleOutOfBoundIndex, 
		OutArrayIndex
	);

	if (ClientStateBuffer.IsIndexInRange(OutArrayIndex))
	{
		return ClientStateBuffer[OutArrayIndex];
	}
	else
	{
		return FSavedClientRigidBodyState::InvalidState;
	}
}

void FNPS_ClientActorPrediction::ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex)
{
	if (!bIsCorrectedStateIndexTooOld)
	{
		int32 ToVerifyOldIndex;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			LastCorrectedStateTickIndex,
			ClientTickIndex,
			ToVerifyOldIndex
		);

		// Ignore old correction.
		if (ToVerifyOldIndex < 0)
		{
			return;
		}
	}

	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(ClientStateBufferStartTickIndex, ClientTickIndex, OutArrayIndex);
	float SumSqrError = 0;
	if (ClientStateBuffer.Num() > 0 && 
		OutArrayIndex >= 0 && 
		OutArrayIndex < ClientStateBuffer.Num())
	{
		FSavedClientRigidBodyState& ExistState = ClientStateBuffer[OutArrayIndex];
		SumSqrError = ExistState.CalculateSumDiffSqrtError(CorrectState);
		ExistState.SaveReplicatedRigidBodyState(CorrectState);
	}
	else
	{
		SumSqrError = TNumericLimits<float>::Max();
		FNPS_StaticHelperFunction::SetElementToBuffers
		(
			ClientStateBuffer, 
			FSavedClientRigidBodyState(CorrectState),
			ClientStateBufferStartTickIndex,
			ClientTickIndex
		);
	}

	bNeedReplay = SumSqrError > 1.0f;
	bIsCorrectedStateIndexTooOld = false;
	LastCorrectedStateTickIndex = ClientTickIndex;
}

void FNPS_ClientActorPrediction::ShiftElementsToDifferentTickIndex(int32 ShiftAmount)
{
	ClientStateBufferStartTickIndex += ShiftAmount;
	LastCorrectedStateTickIndex += ShiftAmount;
}

void FNPS_ClientActorPrediction::ConsumeReplayFlag()
{
	bNeedReplay = false;
}

void FNPS_ClientActorPrediction::Update(uint32 CurrentTickIndex)
{
	if (!bIsCorrectedStateIndexTooOld)
	{
		bIsCorrectedStateIndexTooOld = FNPS_StaticHelperFunction::
			IsCachTickTooOld(LastCorrectedStateTickIndex, CurrentTickIndex);
	}
}

