
// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "NPS_PawnBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "NPS_MovementComponent.h"


// Sets default values
ANPS_PawnBase::ANPS_PawnBase(const FObjectInitializer& OI)
{
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



