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


void UNPS_MovementComponent::SimulatedInput(FVector MoveSpeedVecParam)
{
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
	SimulatedInput(MoveSpeedVec);
}

void UNPS_MovementComponent::TickPostPhysStep(const FPostPhysStepParam& param)
{

}

void UNPS_MovementComponent::TickEndPhysic(const FEndPhysParam& param)
{
	FBodyInstance* BodyInstance = UpdatedPrimitive->GetBodyInstance();
	PxRigidDynamic* RigidBody = BodyInstance->GetPxRigidDynamic_AssumesLocked();
	FVector CurrentVelocity = P2UVector(RigidBody->getLinearVelocity());
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Cyan,
			FString::Printf(TEXT("Velocity=%f"), CurrentVelocity.Size2D()));
	}
}
#pragma endregion PhysicTick

#pragma region ReplayPhys

void UNPS_MovementComponent::TickReplayStart(const FReplayStartParam& param) 
{
	
}

void UNPS_MovementComponent::TickReplaySubstep(const FReplaySubstepParam& param) 
{
	
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

bool UNPS_MovementComponent::TryGetReplayIndex(uint32& OutTickIndex) const
{
	return false;
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

#pragma region INetworkPredictionInterface
void UNPS_MovementComponent::SendClientAdjustment()
{

}

void UNPS_MovementComponent::ForcePositionUpdate(float DeltaTime)
{

}

void UNPS_MovementComponent::SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation, const FVector& NewLocation, const FQuat& NewRotation)
{
	/* We don't use this. 
	*  Smoothing is happened through INetPhysSync.
	*/
}

FNetworkPredictionData_Client* UNPS_MovementComponent::GetPredictionData_Client() const
{
	return nullptr;
}

FNetworkPredictionData_Server* UNPS_MovementComponent::GetPredictionData_Server() const
{
	return nullptr;
}

bool UNPS_MovementComponent::HasPredictionData_Client() const
{
	return false;
}

bool UNPS_MovementComponent::HasPredictionData_Server() const
{
	return false;
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



