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
	SetClientBuffsState(FSavedClientRigidBodyState(PxRigidBodyDynamic), ClientTickIndex);
}

void FNPS_ClientActorPrediction::RetrieveRigidBodyState(physx::PxRigidDynamic* PxRigidDynamic, uint32 ClientTickIndex) const
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(ClientBufferStartTickIndex, ClientTickIndex, 
		OutArrayIndex);

	if (OutArrayIndex >= 0 && OutArrayIndex < ClientStateBuffers.Num())
	{
		ClientStateBuffers[OutArrayIndex].RetriveRigidBodyState(PxRigidDynamic);
	}
}

void FNPS_ClientActorPrediction::ServerCorrectState(const FReplicatedRigidBodyState& CorrectState, uint32 ClientTickIndex)
{
	int32 OutArrayIndex;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex(ClientBufferStartTickIndex, ClientTickIndex, OutArrayIndex);
	if (OutArrayIndex >= 0 && OutArrayIndex < ClientStateBuffers.Num())
	{
		ClientStateBuffers[OutArrayIndex].SaveReplicatedRigidBodyState(CorrectState);
	}
	else
	{
		SetClientBuffsState(FSavedClientRigidBodyState(CorrectState), ClientTickIndex);
	}

	bHasReplayTickIndex = true;
	ReplayTickIndex = ClientTickIndex;
}

void FNPS_ClientActorPrediction::ShiftStartBufferIndex(int32 ShiftAmount)
{
	ClientBufferStartTickIndex += ShiftAmount;
	ReplayTickIndex += ShiftAmount;
}

bool FNPS_ClientActorPrediction::TryGetReplayTickIndex(uint32& OutTickIndex) const
{
	OutTickIndex = ReplayTickIndex;
	return bHasReplayTickIndex;
}

void FNPS_ClientActorPrediction::ClearReplayFlag()
{
	bHasReplayTickIndex = false;
}

bool FNPS_ClientActorPrediction::HasClientBufferYet() const
{
	return ClientStateBuffers.Num() > 0;
}

/**
 * Think of better implementation.
 */
void FNPS_ClientActorPrediction::SetClientBuffsState(const FSavedClientRigidBodyState& ToSet, uint32 ClientTickIndex)
{
	if (HasClientBufferYet())
	{
		int32 OutArrayIndex;
		FNPS_StaticHelperFunction::CalculateBufferArrayIndex(ClientBufferStartTickIndex, ClientTickIndex, OutArrayIndex);

		if (OutArrayIndex >= 0 && OutArrayIndex < ClientStateBuffers.Num())
		{
			ClientStateBuffers[OutArrayIndex] = ToSet;
		}
		else if (OutArrayIndex >= ClientStateBuffers.Num())
		{
			checkf(OutArrayIndex == ClientStateBuffers.Num(), TEXT("Why set array index very far in future?"));

			if (OutArrayIndex == ClientStateBuffers.Num())
			{
				ClientStateBuffers.Add(ToSet);
			}
			else
			{
				ClientStateBuffers.AddDefaulted(OutArrayIndex - ClientStateBuffers.Num() + 1);
				ClientStateBuffers[OutArrayIndex] = ToSet;
			}

		}
		else if (OutArrayIndex < 0)
		{
			ClientBufferStartTickIndex += OutArrayIndex;
			ClientStateBuffers.InsertDefaulted(0, -OutArrayIndex);
			ClientStateBuffers[OutArrayIndex] = ToSet;
		}
	}
	else
	{
		ClientBufferStartTickIndex = ClientTickIndex;
		ClientStateBuffers.Add(ToSet);
	}
}
