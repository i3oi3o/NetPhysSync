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
	bool bUseNearestIfOutOfBound /*=true*/
) const
{
	const FSavedClientRigidBodyState& RetrivedState = GetRigidBodyState(ClientTickIndex, bUseNearestIfOutOfBound);
	// Don't worry.
	// If state is invalid, FSavedClientRigidBodyState::RetriveBodyState() 
	// doesn't copy state to PxRigidDynamic.
	RetrivedState.GetReplicatedRigidBodyState(PxRigidDynamic);
}

const FSavedClientRigidBodyState& FNPS_ClientActorPrediction::GetRigidBodyState
(
	uint32 ClientTickIndex, 
	bool bUseNearestIfTickOutOfRange /*= true*/
) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(ClientStateBufferStartTickIndex, ClientTickIndex,
		OutArrayIndex);

	if (bUseNearestIfTickOutOfRange)
	{
		ClientStateBuffer.ClampIndexParamWithinRange(OutArrayIndex);
	}

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

		UE_LOG
		(
			LogTemp, Log, 
			TEXT("ClientTickIndex:%d, LastCorrectedStateTickIndex:%d, ToVerifyOldIndex:%d"),
			ClientTickIndex,
			LastCorrectedStateTickIndex,
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
	if (OutArrayIndex >= 0 && OutArrayIndex < ClientStateBuffer.Num())
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

bool FNPS_ClientActorPrediction::IsReplayTickIndex(uint32 TickIndex) const
{
	return bNeedReplay && LastCorrectedStateTickIndex == TickIndex;
}

bool FNPS_ClientActorPrediction::TryGetReplayTickIndex(uint32& OutTickIndex) const
{
	OutTickIndex = LastCorrectedStateTickIndex;
	return bNeedReplay;
}



bool FNPS_ClientActorPrediction::TryGetLastCorrectStateTickIndex(uint32& OutTickIndex) const
{
	OutTickIndex = LastCorrectedStateTickIndex;
	return !bIsCorrectedStateIndexTooOld;
}

void FNPS_ClientActorPrediction::ConsumeReplayFlag()
{
	bNeedReplay = false;
}

bool FNPS_ClientActorPrediction::HasClientStateBuffer() const
{
	return ClientStateBuffer.Num() > 0;
}

FBufferInfo FNPS_ClientActorPrediction::GetStateBufferInfo() const
{
	return FBufferInfo(ClientStateBufferStartTickIndex, ClientStateBuffer.Num());
}

void FNPS_ClientActorPrediction::Update(uint32 CurrentTickIndex)
{
	if (!bIsCorrectedStateIndexTooOld)
	{
		int32 Diff;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			LastCorrectedStateTickIndex, 
			CurrentTickIndex,
			Diff
		);

		int32 DiffLimit = TNumericLimits<int32>::Max()-10;
		bIsCorrectedStateIndexTooOld = FMath::Abs(Diff) > DiffLimit;
	}
}

