// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ClientActorPrediction.h"
#include "FNPS_StaticHelperFunction.h"

FNPS_ClientActorPrediction::FNPS_ClientActorPrediction()
{
}

FNPS_ClientActorPrediction::~FNPS_ClientActorPrediction()
{
}

void FNPS_ClientActorPrediction::SaveRigidBodyState(physx::PxRigidDynamic* PxRigidBodyDynamic, uint32 ClientTickIndex)
{
	FNPS_StaticHelperFunction::SetBuffers
	(
		ClientStateBuffers,
		FSavedClientRigidBodyState(PxRigidBodyDynamic),
		ClientStateBufferStartsTickIndex,
		ClientTickIndex
	);
}

void FNPS_ClientActorPrediction::RetrieveRigidBodyState(physx::PxRigidDynamic* PxRigidDynamic, uint32 ClientTickIndex) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(ClientStateBufferStartsTickIndex, ClientTickIndex, 
		OutArrayIndex);

	if (OutArrayIndex >= 0 && OutArrayIndex < ClientStateBuffers.Num())
	{
		ClientStateBuffers[OutArrayIndex].RetriveRigidBodyState(PxRigidDynamic);
	}
}

void FNPS_ClientActorPrediction::ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex)
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(ClientStateBufferStartsTickIndex, ClientTickIndex, OutArrayIndex);
	if (OutArrayIndex >= 0 && OutArrayIndex < ClientStateBuffers.Num())
	{
		ClientStateBuffers[OutArrayIndex].SaveReplicatedRigidBodyState(CorrectState);
	}
	else
	{
		FNPS_StaticHelperFunction::SetBuffers
		(
			ClientStateBuffers, 
			FSavedClientRigidBodyState(CorrectState),
			ClientStateBufferStartsTickIndex,
			ClientTickIndex
		);
	}

	bHasReplayTickIndex = true;
	ReplayTickIndex = ClientTickIndex;
}

void FNPS_ClientActorPrediction::ShiftStartBufferIndex(int32 ShiftAmount)
{
	ClientStateBufferStartsTickIndex += ShiftAmount;
	ReplayTickIndex += ShiftAmount;
}

bool FNPS_ClientActorPrediction::TryGetReplayTickIndex(uint32& OutTickIndex) const
{
	OutTickIndex = ReplayTickIndex;
	return bHasReplayTickIndex;
}

void FNPS_ClientActorPrediction::ConsumeReplayFlag()
{
	bHasReplayTickIndex = false;
}

void FNPS_ClientActorPrediction::TrimBufferToReplayIndex()
{
	if (bHasReplayTickIndex)
	{
		int32 OutArrayIndex;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(ClientStateBufferStartsTickIndex, ReplayTickIndex, OutArrayIndex);

		if (OutArrayIndex > 0)
		{
			ClientStateBuffers.RemoveAt(0, ReplayTickIndex);
			ClientStateBufferStartsTickIndex = ReplayTickIndex;
			
		}
	}
}

bool FNPS_ClientActorPrediction::HasClientStateBufferYet() const
{
	return ClientStateBuffers.Num() > 0;
}


