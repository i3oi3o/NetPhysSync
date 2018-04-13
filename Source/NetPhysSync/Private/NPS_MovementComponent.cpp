// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "NPS_MovementComponent.h"
#include "NPS_BoxPawn.h"
#include <Engine/World.h>
#include "NPSGameState.h"
#include <Components/PrimitiveComponent.h>
#include "PhysXPublic.h"
#include <Engine/Engine.h>
#include <Color.h>

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
	AutoRegisterTick.StartAutoRegister(this);
	// ...
	ANPS_BoxPawn* Owner = Cast<ANPS_BoxPawn>(GetOwner());
	if (Owner != nullptr)
	{
		UpdatedComponent = Owner->GetPhysRootComp();
		UpdatedPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent);
		ForSmoothVisualComponent = Owner->GetForSmoothingVisualComp();
		UpdatedPrimitive->SetPhysicsMaxAngularVelocityInDegrees(MaxAngularVelocityDegree);
	}
}


bool UNPS_MovementComponent::IsTickEnabled() const
{
	AActor* Owner = GetOwner();
	return UpdatedPrimitive != nullptr && 
		!IsPendingKill() && 
		Owner != nullptr && 
		Owner->GetWorld() != nullptr;
}

void UNPS_MovementComponent::TickStartPhysic(const FStartPhysParam& param)
{
	FVector InputVector = this->ConsumeInputVector();

	float InputSize = InputVector.Size();
	MoveSpeed = FMath::Min(InputSize*Speed, Speed);
	
	if (MoveSpeed > 0.01f)
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
	if (MoveSpeed > 0.01f)
	{
		FBodyInstance* BodyInstance = UpdatedPrimitive->GetBodyInstance();
		PxRigidDynamic* RigidBody = BodyInstance->GetPxRigidDynamic_AssumesLocked();
		FVector CurrentVelocity = P2UVector(RigidBody->getLinearVelocity());
		FVector DiffVector = MoveSpeedVec - CurrentVelocity;
		
		
		float DiffSizeSqr = DiffVector.SizeSquared2D();
		if (DiffSizeSqr > 0.01f)
		{
			float Mass = RigidBody->getMass();
			FVector ToAddImpulse;
			DiffVector.Z = 0;
			if (DiffSizeSqr < MaxVelocityChange*MaxVelocityChange)
			{
				ToAddImpulse = Mass*DiffVector;
			}
			else
			{
				ToAddImpulse = Mass*MaxVelocityChange*DiffVector / FMath::Sqrt(DiffSizeSqr);
			}
			
			PxRigidBodyExt::addForceAtLocalPos(*RigidBody, U2PVector(ToAddImpulse),
				U2PVector(ApplyForceLocalPos), PxForceMode::eIMPULSE);
		}
	}
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

void UNPS_MovementComponent::BeginDestroy()
{
	Super::BeginDestroy();
	AutoRegisterTick.StopAutoRegister();
	
	UWorld* World = GetWorld();

	if (World != nullptr)
	{
		ANPSGameState* GameState = World->GetGameState<ANPSGameState>();
		if (GameState != nullptr)
		{
			GameState->UnregisterINetPhysSync(this);
		}
	}
	
}

// Called every frame
void UNPS_MovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
