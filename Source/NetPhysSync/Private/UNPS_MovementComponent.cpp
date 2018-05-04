// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "UNPS_MovementComponent.h"
#include "ANPS_BoxPawn.h"
#include <Engine/World.h>
#include "ANPSGameState.h"
#include <Components/PrimitiveComponent.h>
#include "PhysXPublic.h"
#include <Engine/Engine.h>
#include <Color.h>
#include "FNPS_StaticHelperFunction.h"
#include "FNPS_ClientPawnPrediction.h"
#include "FNPS_ServerPawnPrediction.h"

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
}


// Called when the game starts
void UNPS_MovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	ANPS_BoxPawn* Owner = Cast<ANPS_BoxPawn>(GetOwner());
	if (Owner != nullptr)
	{
		// Is there better way than this?
		AutoRegisterTick.StartAutoRegister(this);
		UpdatedComponent = Owner->GetPhysRootComp();
		UpdatedPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent);
		ForSmoothVisualComponent = Owner->GetForSmoothingVisualComp();
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

bool UNPS_MovementComponent::IsTickEnabled(const FIsTickEnableParam& param) const
{
	AActor* Owner = GetOwner();
	return param.SceneType == EPhysicsSceneType::PST_Sync &&
		UpdatedPrimitive != nullptr &&
		UpdatedPrimitive->Mobility == EComponentMobility::Movable &&
		!IsPendingKill() &&
		Owner != nullptr &&
		Owner->GetWorld() != nullptr;
}


#pragma region PhysicTick


void UNPS_MovementComponent::TickStartPhysic(const FStartPhysParam& param)
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

void UNPS_MovementComponent::TickPhysStep(const FPhysStepParam& param)
{
	FNPS_ClientPawnPrediction* ClientPrecition = 
		static_cast<FNPS_ClientPawnPrediction*>(GetPredictionData_Client());
	ClientPrecition->SaveInput(MoveSpeedVec, param.LocalPhysTickIndex);
	FBodyInstance* BodyInstance = UpdatedPrimitive->GetBodyInstance();
	PxRigidDynamic* RigidDynamic = BodyInstance->GetPxRigidDynamic_AssumesLocked();
	ClientPrecition->SaveRigidBodyState(RigidDynamic, param.LocalPhysTickIndex);
	SimulatedInput(ClientPrecition->GetSavedInput(param.LocalPhysTickIndex));
}

void UNPS_MovementComponent::TickPostPhysStep(const FPostPhysStepParam& param)
{

}

void UNPS_MovementComponent::TickEndPhysic(const FEndPhysParam& param)
{

}
#pragma endregion PhysicTick

#pragma region ReplayPhys

void UNPS_MovementComponent::TickReplayStart(const FReplayStartParam& param) 
{
	FBodyInstance* BodyInstance = UpdatedPrimitive->GetBodyInstance();
	PxRigidDynamic* RigidDynamic = BodyInstance->GetPxRigidDynamic_AssumesLocked();

	FNPS_ClientPawnPrediction* ClientPrediction =
		static_cast<FNPS_ClientPawnPrediction*>(GetPredictionData_Client());
	ClientPrediction->GetRigidBodyState(RigidDynamic, param.StartReplayTickIndex);
	
	// Initialize smooth visual logic here later.
}

void UNPS_MovementComponent::TickReplaySubstep(const FReplaySubstepParam& param) 
{
	FBodyInstance* BodyInstance = UpdatedPrimitive->GetBodyInstance();
	PxRigidDynamic* RigidDynamic = BodyInstance->GetPxRigidDynamic_AssumesLocked();

	FNPS_ClientPawnPrediction* ClientPrediction =
		static_cast<FNPS_ClientPawnPrediction*>(GetPredictionData_Client());
	if (ClientPrediction->IsReplayTickIndex(param.ReplayTickIndex))
	{
		ClientPrediction->GetRigidBodyState(RigidDynamic, param.ReplayTickIndex);
	}

	// Override exist tick.
	ClientPrediction->SaveRigidBodyState(RigidDynamic, param.ReplayTickIndex);

	SimulatedInput(ClientPrediction->GetSavedInput(param.ReplayTickIndex));
}

void UNPS_MovementComponent::TickReplayPostSubstep(const FReplayPostStepParam& param) 
{
	
}

void UNPS_MovementComponent::TickReplayEnd(const FReplayEndParam& param)
{
	
}

#pragma endregion ReplayPhys

void UNPS_MovementComponent::VisualUpdate(const FVisualUpdateParam& param)
{
	
}

bool bHasLastReplayTickIndex;
uint32 LastReplayTickIndex;
bool UNPS_MovementComponent::TryGetReplayIndex(uint32& OutTickIndex) const
{
	/**
	 * Currently for manually testing replay.
	 * Replay every 7 ticks.
	 */
	FNPS_ClientPawnPrediction* ClientPrediction = 
		static_cast<FNPS_ClientPawnPrediction*>(GetPredictionData_Client());

	FBufferInfo StateBufferInfo = ClientPrediction->GetStateBufferInfo();

	if (StateBufferInfo.BufferNum > 7)
	{
		uint32 TestReplayTickIndex = StateBufferInfo.BufferLastTickIndex - 7;

		if (bHasLastReplayTickIndex)
		{
			int32 OutDiff;
			FNPS_StaticHelperFunction::CalculateBufferArrayIndex
			(
				LastReplayTickIndex,
				TestReplayTickIndex,
				OutDiff
			);

			if (OutDiff > 7)
			{
				LastReplayTickIndex = TestReplayTickIndex;
				OutTickIndex = TestReplayTickIndex;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			LastReplayTickIndex = TestReplayTickIndex;
			OutTickIndex = TestReplayTickIndex;
			bHasLastReplayTickIndex = true;
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool UNPS_MovementComponent::TryGetNewSyncTick(FTickSyncPoint& OutNewSyncPoint) const
{
	return false;
}

bool UNPS_MovementComponent::IsLocalPlayerControlPawn() const
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn != nullptr && Pawn->IsLocallyControlled();
}

void UNPS_MovementComponent::OnReadReplication(const FOnReadReplicationParam& ReadReplicationParam)
{

}

#pragma region INetworkPredictionInterface
void UNPS_MovementComponent::SendClientAdjustment()
{
	/**
	 * This is periodically call by UNetDriver.
	 */
}

void UNPS_MovementComponent::ForcePositionUpdate(float DeltaTime)
{
	/**
	 * This is call by APlayerController, using FNetworkPredictionData_Server::Timestamp , This is the last received update.,
	 * to determined when we should force update.
	 */
}

void UNPS_MovementComponent::SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation, const FVector& NewLocation, const FQuat& NewRotation)
{
	/* We don't use this. 
	*  Smoothing is happened through INetPhysSync.
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
		UpdatedPrimitive != nullptr &&
		!IsPendingKill();
}

bool UNPS_MovementComponent::HasPredictionData_Server() const
{
	return ServerPawnPrediction != nullptr &&
		UpdatedPrimitive != nullptr &&
		!IsPendingKill();
}

void UNPS_MovementComponent::ResetPredictionData_Client()
{
	/**
	 * Need to investigate what should we do here.
	 */
}

void UNPS_MovementComponent::ResetPredictionData_Server()
{
	/*
	* Need to investigate what should we do here.
	*/
}
#pragma endregion INetworkPredictionInterface

void UNPS_MovementComponent::BeginDestroy()
{
	Super::BeginDestroy();
	AutoRegisterTick.StopAutoRegister();
	FNPS_StaticHelperFunction::UnregisterINetPhySync(this);
}

// Called every frame
void UNPS_MovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}



