
// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "ANPS_PawnBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "UNPS_MovementComponent.h"


// Sets default values
ANPS_PawnBase::ANPS_PawnBase(const FObjectInitializer& OI)
	:Super(OI)
{
	bReplicateMovement = false;
	bReplicates = true;
	PhysRootComp = OI
		.CreateAbstractDefaultSubobject<UPrimitiveComponent>(this, TEXT("RigidBody"));
	RootComponent = PhysRootComp;

	ForSmoothingVisualComp = OI
		.CreateDefaultSubobject<USceneComponent>(this, TEXT("ForSmoothingVisual"));
	ForSmoothingVisualComp->SetupAttachment(RootComponent);

	SpringArmComp = OI
		.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CamSpringArm"));
	SpringArmComp->SetupAttachment(ForSmoothingVisualComp);
	SpringArmComp->TargetArmLength = 600.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CamComp = OI
		.CreateDefaultSubobject<UCameraComponent>(this, TEXT("Camera"));
	CamComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CamComp->bUsePawnControlRotation = false;

	MovementComponent = OI
		.CreateOptionalDefaultSubobject<UNPS_MovementComponent>(this, TEXT("MovementComp"));
	
	if (MovementComponent)
	{
		MovementComponent->UpdatedComponent = PhysRootComp;
		MovementComponent->UpdatedPrimitive = PhysRootComp;
		MovementComponent->ForSmoothVisualComponent = ForSmoothingVisualComp;
	}

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}


UPrimitiveComponent* ANPS_PawnBase::GetPhysRootComp() const
{
	return PhysRootComp;
}

USceneComponent* ANPS_PawnBase::GetForSmoothingVisualComp() const
{
	return ForSmoothingVisualComp;
}

bool ANPS_PawnBase::Server_UpdateAutonomousInput_Validate(FAutonomousProxyInput AutonomousProxyInput)
{
	return true;
}

void ANPS_PawnBase::Server_UpdateAutonomousInput_Implementation(FAutonomousProxyInput AutonomousProxyInput)
{

}

bool ANPS_PawnBase::Client_CorrectStateWithSyncTick_Validate(FAutoProxySyncCorrect Correction)
{
	return true;
}

void ANPS_PawnBase::Client_CorrectStateWithSyncTick_Implementation(FAutoProxySyncCorrect Correction)
{
	
}

bool ANPS_PawnBase::Client_CorrectState_Validate(FAutoProxyCorrect Correction)
{
	return true;
}

void ANPS_PawnBase::Client_CorrectState_Implementation(FAutoProxyCorrect Correction)
{

}


