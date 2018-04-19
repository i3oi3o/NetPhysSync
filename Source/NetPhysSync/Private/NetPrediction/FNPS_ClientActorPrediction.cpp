// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ClientActorPrediction.h"
#include "FNPS_StaticHelperFunction.h"

FNPS_ClientActorPrediction::FNPS_ClientActorPrediction() 
	: ClientStateBuffers(20),
	ClientStateBufferStartsTickIndex(0),
	LastCorrectedStateTickIndex(0),
	bNeedReplay(false)
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
		ClientStateBuffers,
		SaveRigidBodyState,
		ClientStateBufferStartsTickIndex,
		ClientTickIndex
	);
}

void FNPS_ClientActorPrediction::GetRigidBodyState
(
	physx::PxRigidDynamic* PxRigidDynamic, 
	uint32 ClientTickIndex, 
	bool bUseNearestIfTickOutOfRange /*=true*/
) const
{
	FSavedClientRigidBodyState RetrivedState = GetRigidBodyState(ClientTickIndex, bUseNearestIfTickOutOfRange);
	// Don't worry.
	// If state is invalid, FSavedClientRigidBodyState::RetriveBodyState() 
	// doesn't copy state to PxRigidDynamic.
	RetrivedState.RetriveRigidBodyState(PxRigidDynamic);
}

FSavedClientRigidBodyState FNPS_ClientActorPrediction::GetRigidBodyState
(
	uint32 ClientTickIndex, 
	bool bUseNearestIfTickOutOfRange /*= true*/
) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(ClientStateBufferStartsTickIndex, ClientTickIndex,
		OutArrayIndex);

	if (bUseNearestIfTickOutOfRange)
	{
		if (OutArrayIndex < 0)
		{
			OutArrayIndex = 0;
		}
		else if(OutArrayIndex < ClientStateBuffers.Num())
		{
			OutArrayIndex = ClientStateBuffers.Num() - 1;
		}
	}

	if (ClientStateBuffers.IsIndexInRange(OutArrayIndex))
	{
		return ClientStateBuffers[OutArrayIndex];
	}
	else
	{
		return FSavedClientRigidBodyState();
	}
}

void FNPS_ClientActorPrediction::ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex)
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(ClientStateBufferStartsTickIndex, ClientTickIndex, OutArrayIndex);
	float SumSqrError = 0;
	if (OutArrayIndex >= 0 && OutArrayIndex < ClientStateBuffers.Num())
	{
		FSavedClientRigidBodyState& ExistState = ClientStateBuffers[OutArrayIndex];
		SumSqrError = ExistState.CalculatedSumDiffSqurError(CorrectState);
		ExistState.SaveReplicatedRigidBodyState(CorrectState);
	}
	else
	{
		SumSqrError = TNumericLimits<float>::Max();
		FNPS_StaticHelperFunction::SetElementToBuffers
		(
			ClientStateBuffers, 
			FSavedClientRigidBodyState(CorrectState),
			ClientStateBufferStartsTickIndex,
			ClientTickIndex
		);
	}

	bNeedReplay = SumSqrError > 1.0f;
	LastCorrectedStateTickIndex = ClientTickIndex;
}

void FNPS_ClientActorPrediction::ShiftStartBufferIndex(int32 ShiftAmount)
{
	ClientStateBufferStartsTickIndex += ShiftAmount;
	LastCorrectedStateTickIndex += ShiftAmount;
}

bool FNPS_ClientActorPrediction::TryGetReplayTickIndex(uint32& OutTickIndex) const
{
	OutTickIndex = LastCorrectedStateTickIndex;
	return bNeedReplay;
}

void FNPS_ClientActorPrediction::ConsumeReplayFlag()
{
	bNeedReplay = false;
}

bool FNPS_ClientActorPrediction::HasClientStateBufferYet() const
{
	return ClientStateBuffers.Num() > 0;
}


