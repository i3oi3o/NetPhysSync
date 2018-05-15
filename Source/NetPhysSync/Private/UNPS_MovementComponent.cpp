// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "UNPS_MovementComponent.h"
#include "NPSLogCategory.h"
#include "ANPS_PawnBase.h"
#include <Engine/World.h>
#include "ANPSGameState.h"
#include <Components/PrimitiveComponent.h>
#include "PhysXPublic.h"
#include <Engine/Engine.h>
#include <Color.h>
#include "FNPS_StaticHelperFunction.h"
#include "FNPS_ClientPawnPrediction.h"
#include "FNPS_ServerPawnPrediction.h"
#include "DrawDebugHelpers.h"
#include "INetPhysSyncParam.h"

#if !(UE_BUILD_SHIPPING)
#include "Components/BoxComponent.h"
#endif

using namespace physx;

// Sets default values for this component's properties
UNPS_MovementComponent::UNPS_MovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	Speed = 1000.0f;
	MaxVelocityChange = 500.0f;
	ApplyForceLocalPos = FVector(0.0f, 0.0f, 10.0f);
	MaxAngularVelocityDegree = 90.0f;
	bServerHasAutoProxyPendingCorrection = false;
	bClientHasNewSyncPoint = false;
	bClientHasNewServerTick = false;
    bClientHasAutoCorrectWithoutSyncTick = false;
	bClientHasServerTick = false;
	bClientHasSyncPoint = false;
}


bool UNPS_MovementComponent::IsComponentDataValid() const
{
	return UpdatedPrimitive != nullptr &&
		UpdatedPrimitive->Mobility == EComponentMobility::Movable &&
		!IsPendingKill() &&
		NPS_PawnOwner != nullptr &&
		NPS_PawnOwner->GetWorld() != nullptr;
}



// Called when the game starts
void UNPS_MovementComponent::BeginPlay()
{
	Super::BeginPlay();
	PawnOwner = Cast<APawn>(GetOwner());
	NPS_PawnOwner = Cast<ANPS_PawnBase>(PawnOwner);

	// ...
	if (NPS_PawnOwner != nullptr)
	{
		FNPS_StaticHelperFunction::RegisterINetPhySync(this);
		UpdatedComponent = NPS_PawnOwner->GetPhysRootComp();
		UpdatedPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent);
		ForSmoothVisualComponent = NPS_PawnOwner->GetForSmoothingVisualComp();
		UpdatedPrimitive->SetPhysicsMaxAngularVelocityInDegrees(MaxAngularVelocityDegree);
	}
}


void UNPS_MovementComponent::SimulatedInput(const FSavedInput& SavedInput)
{
	FVector MoveSpeedVecParam = SavedInput.GetTargetSpeed();
	float CurrentSpeedSizeSqrt = MoveSpeedVecParam.SizeSquared2D();
	if (CurrentSpeedSizeSqrt > 0.0001f)
	{
		float CurrentSpeedSize = FMath::Sqrt(CurrentSpeedSizeSqrt);
		FVector MoveDir = MoveSpeedVecParam / CurrentSpeedSize;
		FBodyInstance* BodyInstance = UpdatedPrimitive->GetBodyInstance();
		PxRigidDynamic* RigidBody = BodyInstance->GetPxRigidDynamic_AssumesLocked();
		FVector CurrentVelocity = P2UVector(RigidBody->getLinearVelocity());
		FVector DiffVector = MoveSpeedVecParam - CurrentVelocity;
		
		// Remove projected negative from DiffVector
		float DotProduct = FVector::DotProduct(DiffVector, MoveDir);
		if (DotProduct < 0)
		{
			DiffVector -= MoveDir*DotProduct;
		}

		float DiffSizeSqr = DiffVector.SizeSquared2D();
		if (DiffSizeSqr > 0.01f)
		{
			float Mass = RigidBody->getMass();
			FVector ToAddImpulse;
			DiffVector.Z = 0;
			float CurrentMaxVelocityChange = CurrentSpeedSize*MaxVelocityChange / Speed;


			if (DiffSizeSqr < CurrentMaxVelocityChange*CurrentMaxVelocityChange)
			{
				ToAddImpulse = Mass*DiffVector;
			}
			else
			{
				ToAddImpulse = Mass*CurrentMaxVelocityChange
					*DiffVector 
					/ FMath::Sqrt(DiffSizeSqr);
			}

			PxRigidBodyExt::addForceAtLocalPos(*RigidBody, U2PVector(ToAddImpulse),
				U2PVector(ApplyForceLocalPos), PxForceMode::eIMPULSE);
		}
	}
}

FNPS_ClientPawnPrediction* UNPS_MovementComponent::GetPredictionData_ClientNPSPawn() const
{
	return static_cast<FNPS_ClientPawnPrediction*>(GetPredictionData_Client());
}

FNPS_ServerPawnPrediction* UNPS_MovementComponent::GetPredictionData_ServerNPSPawn() const
{
	return static_cast<FNPS_ServerPawnPrediction*>(GetPredictionData_Server());
}



physx::PxRigidDynamic* UNPS_MovementComponent::GetUpdatedRigidDynamic()
{
	FBodyInstance* BodyInstance = UpdatedPrimitive->GetBodyInstance();
	return BodyInstance->GetPxRigidDynamic_AssumesLocked();
}

bool UNPS_MovementComponent::IsLateSyncClientTick
(
	const FNPS_ClientPawnPrediction* ClientPrediction, 
	const FAutoProxySyncCorrect& SyncCorrect
) const
{
	// See UNPS_MovementComponent::Client_CorrectStateWithSyncTick_Implementation(const FAutoProxySyncCorrect&)
	// for comment about this implementation here.
	FBufferInfo BufferInfo = ClientPrediction->GetInputBufferInfo();

	bool bIsLateSyncClientTick = true;
	uint32 QueryLastProcessedInput;

	if (BufferInfo.BufferNum == 0)
	{
		bIsLateSyncClientTick = false;
	}
	else if (SyncCorrect.TryGetLastProcessedClientInputTick(QueryLastProcessedInput))
	{
		int32 Index = FNPS_StaticHelperFunction::
			CalculateBufferArrayIndex
			(
				BufferInfo.BufferStartTickIndex,
				QueryLastProcessedInput
			);

		bIsLateSyncClientTick = Index < 0;
	}

	return bIsLateSyncClientTick;
}

bool UNPS_MovementComponent::IsReceivedServerTickTooOld(uint32 ServerTick)
{
	if (bClientHasServerTick)
	{
		int32 Index = FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(
			ClientReceivedServerTick,
			ServerTick
		);

		return (Index < 0);
	}

	return false;
}

void UNPS_MovementComponent::ResetClientCachNewDataFlag()
{
	bClientHasNewSyncPoint = false;
	bClientHasNewServerTick = false;
	bClientHasAutoCorrectWithoutSyncTick = false;
}

void UNPS_MovementComponent::DrawDebugRigidBody
(
	const FReplicatedRigidBodyState& DrawPos, 
	const FColor& Color
)
{
#if !(UE_BUILD_SHIPPING)
	UBoxComponent* BoxComponent = Cast<UBoxComponent>(UpdatedPrimitive);

	if (BoxComponent != nullptr)
	{
		DrawDebugBox
		(
			this->GetWorld(),
			DrawPos.GetWorldPos(),
			BoxComponent->GetScaledBoxExtent(),
			DrawPos.GetWorldRotation(),
			Color, false, 0.125f
		);
	}

#endif
}

// ----------------------- Start INetPhysSync ----------------------

bool UNPS_MovementComponent::IsTickEnabled(const FIsTickEnableParam& param) const
{
	AActor* Owner = GetOwner();
	return param.SceneType == EPhysicsSceneType::PST_Sync &&
		IsComponentDataValid();
}


// ------------------------- Start INetPhysSync:: Tick interface -------------
void UNPS_MovementComponent::TickStartPhysic(const FStartPhysParam& param)
{
	bStartPhysicYet = true;
	if (IsLocalPlayerControlPawn())
	{
		FVector InputVector = this->ConsumeInputVector();

		float InputSize = InputVector.Size();
		MoveSpeed = FMath::Min(InputSize*Speed, Speed);

		if (InputSize > 0.05f) // Death Zone for Controller.
		{
			FVector InputDir = InputVector / InputSize;
			InvMoveSpeed = 1.0f / MoveSpeed;
			MoveSpeedVec = InputDir * MoveSpeed;
		}
		else
		{
			InvMoveSpeed = 0.0f;
			MoveSpeedVec = FVector(0.0f, 0.0f, 0.0f);
		}
	}
}

void UNPS_MovementComponent::TickPhysStep(const FPhysStepParam& param)
{
	FBodyInstance* BodyInstance = UpdatedPrimitive->GetBodyInstance();
	PxRigidDynamic* RigidDynamic = BodyInstance->GetPxRigidDynamic_AssumesLocked();

	if (IsLocalPlayerControlPawn())
	{
		if (PawnOwner->IsNetMode(NM_Standalone))
		{
			SimulatedInput(FSavedInput(MoveSpeedVec));
		}
		else
		{
			FNPS_ClientPawnPrediction* ClientPrecition = GetPredictionData_ClientNPSPawn();
			ClientPrecition->Update(param.LocalPhysTickIndex);
			ClientPrecition->SaveInput(MoveSpeedVec, param.LocalPhysTickIndex);
			ClientPrecition->SaveRigidBodyState(RigidDynamic, param.LocalPhysTickIndex);
			SimulatedInput(ClientPrecition->GetSavedInput(param.LocalPhysTickIndex));
		}
	}
	else if (PawnOwner->Role == ENetRole::ROLE_Authority)
	{
		// Handle Server Logic here.
		ensureMsgf
		(
			PawnOwner->GetNetMode() == ENetMode::NM_DedicatedServer ||
			PawnOwner->GetNetMode() == ENetMode::NM_ListenServer,
			TEXT("This is not server. Why is the code hit here?")
		);

		FNPS_ServerPawnPrediction* ServerPrediction = 
			GetPredictionData_ServerNPSPawn();

#if NPS_LOG_SYNC_AUTO_PROXY
		if (!ServerPrediction->IsProcessingClientInput())
		{
			if (ServerPrediction->HasSyncClientTickIndex())
			{
				UE_LOG
				(
					LogNPS_Net, Log, TEXT("Process ServerTick:%u, ClientTick:%u"),
					param.LocalPhysTickIndex,
					ServerPrediction->GetSyncClientTickIndex(param.LocalPhysTickIndex)
				);
			}
			else
			{
				UE_LOG
				(
					LogNPS_Net, Log, TEXT("Process ServerTick:%u"), param.LocalPhysTickIndex
				);
			}
		}
#endif
		const FSavedInput& ProcessedInput = ServerPrediction->
			ProcessServerTick(param.LocalPhysTickIndex);
		SimulatedInput(ProcessedInput);
	}
	else if (PawnOwner->Role == ENetRole::ROLE_SimulatedProxy)
	{
		// Handle simulated proxy logic here.
	}
}

void UNPS_MovementComponent::TickPostPhysStep(const FPostPhysStepParam& param)
{

}

void UNPS_MovementComponent::TickEndPhysic(const FEndPhysParam& param)
{
	if (IsLocalPlayerControlPawn() && 
		PawnOwner->Role == ENetRole::ROLE_AutonomousProxy)
	{
		FNPS_ClientPawnPrediction* ClientPrediction = GetPredictionData_ClientNPSPawn();

		if (ClientPrediction->HasUnacknowledgedInput())
		{
			FAutonomousProxyInput ProxyInput = FAutonomousProxyInput(*ClientPrediction);
			
#if NPS_LOG_SYNC_AUTO_PROXY
			UE_LOG
			(
				LogNPS_Net, Log, TEXT("Sending - ProxyStartTick:%u, ProxyNum:%u"),
				ProxyInput.GetArrayStartClientTickIndex(),
				ProxyInput.GetArray().Num()
			);
#endif
			Server_UpdateAutonomousInput
			(
				MoveTemp(ProxyInput)
			);
		}
	}
}
// ------------------------- End INetPhysSync:: Tick interface -------------

// ------------------------- Start INetPhysSync:: Replay intgerface --------------------

void UNPS_MovementComponent::TickReplayStart(const FReplayStartParam& param) 
{
	
	if (PawnOwner->Role == ENetRole::ROLE_AutonomousProxy)
	{
		PxRigidDynamic* RigidDynamic = GetUpdatedRigidDynamic();

		FNPS_ClientPawnPrediction* ClientPrediction = GetPredictionData_ClientNPSPawn();
		ClientPrediction->GetRigidBodyState
		(
			RigidDynamic, param.StartReplayTickIndex,
			EIdxOutOfRangeHandle::UseNearestIndexIfOutRangeFromBegin
		);

		// Use our own compilation symbol later.
	}
	else if (PawnOwner->Role == ENetRole::ROLE_SimulatedProxy)
	{
		// Handle simulated proxy later
	}
	
	
	// Initialize smooth visual logic here later.
}

void UNPS_MovementComponent::TickReplaySubstep(const FReplaySubstepParam& param) 
{
	FBodyInstance* BodyInstance = UpdatedPrimitive->GetBodyInstance();
	PxRigidDynamic* RigidDynamic = BodyInstance->GetPxRigidDynamic_AssumesLocked();
	FNPS_ClientPawnPrediction* ClientPrediction =
		static_cast<FNPS_ClientPawnPrediction*>(GetPredictionData_Client());

	if (PawnOwner->Role == ENetRole::ROLE_AutonomousProxy)
	{
		if (ClientPrediction->IsReplayTickIndex(param.ReplayTickIndex))
		{
			ClientPrediction->GetRigidBodyState(RigidDynamic, param.ReplayTickIndex);
		}

		// Override exist saved state.
		ClientPrediction->SaveRigidBodyState(RigidDynamic, param.ReplayTickIndex);

		SimulatedInput(ClientPrediction->GetSavedInput(param.ReplayTickIndex));
	}
	else if (PawnOwner->Role == ENetRole::ROLE_SimulatedProxy)
	{
		// Handle simulated proxy later.
	}
}

void UNPS_MovementComponent::TickReplayPostSubstep(const FReplayPostStepParam& param) 
{
	
}

void UNPS_MovementComponent::TickReplayEnd(const FReplayEndParam& param)
{
	// Reset all the flag here.
	FNPS_ClientPawnPrediction* ClientPrecition =
		static_cast<FNPS_ClientPawnPrediction*>(GetPredictionData_Client());
	ClientPrecition->ConsumeReplayFlag();
}

// ------------------------ End INetPhySync:: Replay interface -----------------------

void UNPS_MovementComponent::VisualUpdate(const FVisualUpdateParam& param)
{
	
}

bool UNPS_MovementComponent::TryGetNewestUnprocessedServerTick(uint32& OutServerTick) const
{
	OutServerTick = ClientReceivedServerTick;
	return bClientHasNewServerTick;
}


bool UNPS_MovementComponent::TryGetReplayIndex(uint32& OutTickIndex) const
{
	FNPS_ClientPawnPrediction* ClientPrediction = GetPredictionData_ClientNPSPawn();
	return ClientPrediction->TryGetReplayTickIndex(OutTickIndex);
}

bool UNPS_MovementComponent::TryGetNewSyncTick(FTickSyncPoint& OutNewSyncPoint) const
{
	if (bClientHasNewSyncPoint)
	{
		OutNewSyncPoint = ClientReceiveTickSyncPoint;
	}
	return bClientHasNewSyncPoint;
}

bool UNPS_MovementComponent::IsLocalPlayerControlPawn() const
{
	return PawnOwner != nullptr && PawnOwner->IsLocallyControlled();
}

void UNPS_MovementComponent::OnReadReplication
(
	const FOnReadReplicationParam& ReadReplicationParam
)
{
	FNPS_ClientPawnPrediction* ClientPrecition = GetPredictionData_ClientNPSPawn();
	int32 ShiftAmount = ReadReplicationParam.NewSyncPointInfo
		.ShiftClientTickAmountForReplayPrediction;


	if (PawnOwner->Role == ENetRole::ROLE_AutonomousProxy)
	{
		if (!bClientHasSyncPoint)
		{
			ClientPrecition->ShiftElementsToDifferentTickIndex(ShiftAmount);
		}

		if (bClientHasAutoCorrectWithoutSyncTick)
		{
			checkf
			(
				ClientAutoProxyCorrectWithoutSyncTick.GetServerTick() == ClientReceivedServerTick,
				TEXT("Receive server tick doesn't match.")
			);

			uint32 ReplayClientTick = ReadReplicationParam
				.NewSyncPointInfo
				.NewSyncPoint
				.ServerTick2ClientTick(ClientReceivedServerTick);

			ClientPrecition->ServerCorrectState
			(
				ClientAutoProxyCorrectWithoutSyncTick.GetRigidBodyState(),
				ReplayClientTick
			);
		}
	}
}

void UNPS_MovementComponent::OnFinishReadReplication(const FOnFinishReadAllReplicationParam& FinishReadRepParam)
{
	ResetClientCachNewDataFlag();
}

// ---------------------- End INetPhysSync --------------------------------------- 

// ---------------------- Start INetworkdPredictionInterface --------------------
void UNPS_MovementComponent::SendClientAdjustment()
{
	checkf(IsComponentDataValid(), TEXT("Component data is not valid anymore why?"));
	/**
	 * This is periodically call by UNetDriver.
	 */
	if (bServerHasAutoProxyPendingCorrection)
	{

		FNPS_ServerPawnPrediction* ServerPrecition = GetPredictionData_ServerNPSPawn();
		// Prevent force update in case that our physic delta time is high.
		ServerPrecition->ServerTimeStamp = GetWorld()->TimeSeconds;

		uint32 ServerTick = FNPS_StaticHelperFunction::GetCurrentPhysTickIndex(this);
		FBufferInfo BufferInfo = ServerPawnPrediction->GetInputServerTickBufferInfo();

		int32 ProcessedInputIndex = FNPS_StaticHelperFunction::
			CalculateBufferArrayIndex(BufferInfo.BufferStartTickIndex, ServerTick);
		
		bool ProcessedInputYet =
			// This condition handle jitter waiting logic. 
			// If we're waiting, don't send any update yet.
			ProcessedInputIndex >= 0 || 
			// if buffer is empty, server already processed all input.
			BufferInfo.BufferNum == 0;

		if (ProcessedInputYet)
		{
			ensureMsgf
			(
				ServerPawnPrediction->HasSyncClientTickIndex(),
				TEXT("We should have sync client tick index.")
			);

			// LastProcessedClientInputTick can be missing if current tick reach warping point.
			if (ServerPrecition->HasLastProcessedClientInputTickIndex())
			{
				FAutoProxySyncCorrect ToSend = FAutoProxySyncCorrect
				(
					FReplicatedRigidBodyState(GetUpdatedRigidDynamic()),
					ServerPrecition->GetSyncClientTickIndex(ServerTick),
					ServerTick,
					ServerPrecition->GetLastProcessedClientInputTickIndex()
				);

#if !UE_BUILD_SHIPPING
				uint32 LastProcessedClientInputTick;
				ensureMsgf
				(
					ToSend.GetSyncServerTick() == ServerTick &&
					ToSend.GetSyncClientTick() == ServerPrecition->GetSyncClientTickIndex(ServerTick) &&
					ToSend.TryGetLastProcessedClientInputTick(LastProcessedClientInputTick) &&
					LastProcessedClientInputTick == ServerPrecition->GetLastProcessedClientInputTickIndex(),
					TEXT("Check typo")
				);
#endif
				Client_CorrectStateWithSyncTick
				(
					ToSend
				);
			}
			else
			{
				FAutoProxySyncCorrect ToSend = FAutoProxySyncCorrect
				(
					FReplicatedRigidBodyState(GetUpdatedRigidDynamic()),
					ServerPrecition->GetSyncClientTickIndex(ServerTick),
					ServerTick
				);

#if !UE_BUILD_SHIPPING
				ensureMsgf
				(
					ToSend.GetSyncServerTick() == ServerTick &&
					ToSend.GetSyncClientTick() == ServerPrecition->GetSyncClientTickIndex(ServerTick),
					TEXT("Check typo")
				);
#endif

				Client_CorrectStateWithSyncTick
				(
					ToSend
				);
			}

			bServerHasAutoProxyPendingCorrection = false;

#if NPS_LOG_SYNC_AUTO_PROXY
			UE_LOG
			(
				LogNPS_Net, Log, TEXT("Send correction with sync tick. ServerTick:%u, ClientTick:%u"),
				ServerTick, ServerPrecition->GetSyncClientTickIndex(ServerTick)
			);
#endif
		}
	}
}

void UNPS_MovementComponent::ForcePositionUpdate(float DeltaTimeSinceLastCorrection)
{
	/**
	 * This is call through APlayerController on server, using FNetworkPredictionData_Server::Timestamp , This is the last received update.,
	 * to determined when we should force update.
	 */
	FNPS_ServerPawnPrediction* ServerPrediction = GetPredictionData_ServerNPSPawn();

	if (ServerPrediction->HasSyncClientTickIndex())
	{
		// Set flag to true. So, SendClientAdjustment sending data to client.
		// Refactor this later.
		bServerHasAutoProxyPendingCorrection = true;
		SendClientAdjustment();
	}
	else
	{
		uint32 ServerTick = FNPS_StaticHelperFunction::GetCurrentPhysTickIndex(this);
		FReplicatedRigidBodyState RigidBodyState(GetUpdatedRigidDynamic());
		Client_CorrectState(FAutoProxyCorrect(RigidBodyState, ServerTick));
	}

}

void UNPS_MovementComponent::SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation, const FVector& NewLocation, const FQuat& NewRotation)
{
	/* We don't use this. 
	*  Smoothing is happened through INetPhysSync interface.
	*/
}

FNetworkPredictionData_Client* UNPS_MovementComponent::GetPredictionData_Client() const
{
	if (ClientPawnPrediction == nullptr)
	{
		UNPS_MovementComponent* MutableThis = const_cast<UNPS_MovementComponent*>(this);
		MutableThis->ClientPawnPrediction = new FNPS_ClientPawnPrediction();
	}
	return ClientPawnPrediction;
}

FNetworkPredictionData_Server* UNPS_MovementComponent::GetPredictionData_Server() const
{
	if (ServerPawnPrediction == nullptr)
	{
		UNPS_MovementComponent* MutableThis = const_cast<UNPS_MovementComponent*>(this);
		MutableThis->ServerPawnPrediction = new FNPS_ServerPawnPrediction
		(
			FNPS_StaticHelperFunction::GetNetSetting()
		);
	}
	return ServerPawnPrediction;
}

bool UNPS_MovementComponent::HasPredictionData_Client() const
{
	return ClientPawnPrediction != nullptr && 
		IsComponentDataValid();
}

bool UNPS_MovementComponent::HasPredictionData_Server() const
{
	return ServerPawnPrediction != nullptr &&
		IsComponentDataValid();
}

void UNPS_MovementComponent::ResetPredictionData_Client()
{
	/**
	 * Need to investigate what should we do here.
	 */
	ResetClientCachNewDataFlag();
	bClientHasServerTick = false;
	bClientHasSyncPoint = false;
}

void UNPS_MovementComponent::ResetPredictionData_Server()
{
	/*
	* Need to investigate what should we do here.
	*/
	bServerHasAutoProxyPendingCorrection = false;
#if NPS_LOG_SYNC_AUTO_PROXY
	UE_LOG(LogNPS_Net, Log, TEXT("ResetPredictionDataServer."));
#endif
}

// ---------------------- End INetworkdPredictionInterface --------------------


// --------------------- Start Re-route RPC Function ----------------------------
void UNPS_MovementComponent::Server_UpdateAutonomousInput(FAutonomousProxyInput&& AutonomousProxyInpit)
{
	checkf(NPS_PawnOwner != nullptr, TEXT("Missing NPS_PawnOwner"));
	NPS_PawnOwner->Server_UpdateAutonomousInput(AutonomousProxyInpit);
}

void UNPS_MovementComponent::Server_UpdateAutonomousInput_Imlementation(const FAutonomousProxyInput& AutonomousProxyInpit)
{
	FNPS_ServerPawnPrediction* ServerPrediction = GetPredictionData_ServerNPSPawn();
	// Need to update this so, ForceUpdate work properly.
	ServerPrediction->ServerTimeStamp = GetWorld()->TimeSeconds;
	
	uint32 ServerTick = FNPS_StaticHelperFunction::GetCurrentPhysTickIndex(this);

	ServerPrediction->UpdateInputBuffer(AutonomousProxyInpit, ServerTick);

	bServerHasAutoProxyPendingCorrection = true;
}

void UNPS_MovementComponent::Client_CorrectStateWithSyncTick(const FAutoProxySyncCorrect& AutoProxySyncCorrect)
{
	checkf(NPS_PawnOwner != nullptr, TEXT("Missing NPS_PawnOwner"));
	NPS_PawnOwner->Client_CorrectStateWithSyncTick(AutoProxySyncCorrect);
}

void UNPS_MovementComponent::Client_CorrectStateWithSyncTick_Implementation
(
	const FAutoProxySyncCorrect& AutoProxySyncCorrect
)
{
	// Ignore old correction from out of order package.
	if (IsReceivedServerTickTooOld(AutoProxySyncCorrect.GetSyncServerTick()))
	{
		return;
	}

	bClientHasSyncPoint = true;
	bClientHasServerTick = true;
	bClientHasNewServerTick = true;
	bClientHasNewSyncPoint = true;

	FNPS_ClientPawnPrediction* ClientPrediction = GetPredictionData_ClientNPSPawn();
	
	bool bIsLateSyncClientTick = IsLateSyncClientTick(ClientPrediction, 
					AutoProxySyncCorrect);
	
	if (!bIsLateSyncClientTick)
	{
#if !UE_BUILD_SHIPPING
		uint32 LocalPhysicTick = FNPS_StaticHelperFunction::GetCurrentPhysTickIndex(this);
		
		int32 Diff = FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(AutoProxySyncCorrect.GetSyncClientTick(), LocalPhysicTick);
		
		ensureMsgf(Diff >= 0, TEXT("Replay into future"));
#endif
		ClientPrediction->ServerCorrectState
		(
			AutoProxySyncCorrect.GetRigidBodyState(),
			AutoProxySyncCorrect.GetSyncClientTick()
		);
		
		ClientReceiveTickSyncPoint = AutoProxySyncCorrect.CreateTickSyncPoint();
	}
	else
	{

		/**
		 * Here if you are coming form comment in UNPS_MovementComponent::
		 * CanUseSyncClientTickWithCurrentInputBuffer(const FNPS_ClientPawnPrediction*, const FAutoProxySyncCorrect&).
		 * 
		 *
		 * Because of latency, the correct state may not processed input in buffer yet.
		 * 
		 * Consider following scenario.
		 *		- Server have processed client input from tick 10-20
		 *		- Server send correction which is state from processed last input tick 20.
		 *		- AutonomousProxy start receive input from player controller at tick 50.
		 *		- AutonomousProxy received correct input state from processing last input tick 20 
		 *		  but not form the starting new steam input tick 50.
		 *		- This scenario create minor discrepancy.
		 *
		 * If this correct state is not for current input buffer, 
		 * we use this to correct state before input buffer start.
		 *
		 * 
		 */
		FBufferInfo BufferInfo = ClientPrediction->GetInputBufferInfo();
		ensureMsgf(BufferInfo.BufferNum > 0, TEXT("Buffer shouldn't be empty."));
		uint32 CorrectClientTick = BufferInfo.BufferStartTickIndex - 1;
		uint32 SyncServerTick = AutoProxySyncCorrect.GetSyncServerTick();

		ClientReceiveTickSyncPoint = FTickSyncPoint(CorrectClientTick, SyncServerTick);

		ensureMsgf
		(
			ClientReceiveTickSyncPoint.GetClientTickSyncPoint() == CorrectClientTick &&
			ClientReceiveTickSyncPoint.GetServerTickSyncPoint() == SyncServerTick,
			TEXT("Typo when create FTickSyncPoint.")
		);

		ClientPrediction->ServerCorrectState
		(
			AutoProxySyncCorrect.GetRigidBodyState(),
			CorrectClientTick
		);

#if !UE_BUILD_SHIPPING
		uint32 LocalPhysicTick = FNPS_StaticHelperFunction::GetCurrentPhysTickIndex(this);	
		int32 Diff = FNPS_StaticHelperFunction::CalculateBufferArrayIndex
		(CorrectClientTick, LocalPhysicTick);

		ensureMsgf(Diff >= 0, TEXT("Replay into future?."));
#endif

#if NPS_LOG_SYNC_AUTO_PROXY
		UE_LOG
		(
			LogNPS_Net, Log, 
			TEXT("Correct with late sync point. Need Replay: %s"),
			ClientPrediction->IsReplayTickIndex(CorrectClientTick) ? 
			TEXT("True") : TEXT("False")
		);
#endif
	}

	ClientReceivedServerTick = AutoProxySyncCorrect.GetSyncServerTick();
	DrawDebugRigidBody(AutoProxySyncCorrect.GetRigidBodyState(), FColor::Blue);
}

void UNPS_MovementComponent::Client_CorrectState(const FAutoProxyCorrect& Correction)
{
	checkf(NPS_PawnOwner != nullptr, TEXT("Missing NPS_PawnOwner"));
	NPS_PawnOwner->Client_CorrectState(Correction);
}

void UNPS_MovementComponent::Client_CorrectState_Implementation
(
	const FAutoProxyCorrect& Correction
)
{
	// Ignore old correction from out of order package.
	if (IsReceivedServerTickTooOld(Correction.GetServerTick()))
	{
		return;
	}

	bClientHasServerTick = true;
	bClientHasNewServerTick = true;
	ClientReceivedServerTick = Correction.GetServerTick();
	bClientHasAutoCorrectWithoutSyncTick = true;
	ClientAutoProxyCorrectWithoutSyncTick = Correction;
	DrawDebugRigidBody(Correction.GetRigidBodyState(), FColor::Blue);
}
// ------------------------ End Re-route RPC Function ----------------------

void UNPS_MovementComponent::BeginDestroy()
{
	Super::BeginDestroy();
	FNPS_StaticHelperFunction::UnregisterINetPhySync(this);

	if (ClientPawnPrediction != nullptr)
	{
		delete ClientPawnPrediction;
		ClientPawnPrediction = nullptr;
	}

	if (ServerPawnPrediction != nullptr)
	{
		delete ServerPawnPrediction;
		ServerPawnPrediction = nullptr;
	}
}

// Called every frame
void UNPS_MovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}



