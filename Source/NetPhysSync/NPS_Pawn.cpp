// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "NPS_Pawn.h"
#include "Components/BoxComponent.h"
#include <Engine/CollisionProfile.h>
#include <Components/StaticMeshComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include <ConstructorHelpers.h>
#include "NPS_MovementComponent.h"


// Sets default values
ANPS_Pawn::ANPS_Pawn()
{
	UBoxComponent* BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RigidBody"));
	BoxComponent->InitBoxExtent(FVector(50.0f));
	BoxComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	BoxComponent->bDynamicObstacle = true;
	BoxComponent->bCheckAsyncSceneOnMove = false;
	BoxComponent->bShouldUpdatePhysicsVolume = true;
	BoxComponent->SetSimulatePhysics(true);
	PhysRootComp = BoxComponent;
	RootComponent = BoxComponent;

	ForSmoothingVisualComp = CreateDefaultSubobject<USceneComponent>(TEXT("ForSmoothingVisual"));
	ForSmoothingVisualComp->SetupAttachment(RootComponent);

	UStaticMeshComponent* BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	BoxMesh->SetupAttachment(ForSmoothingVisualComp);
	BoxMesh->SetSimulatePhysics(false);
	BoxMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	VisualComp = BoxMesh;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CamSpringArm"));
	SpringArmComp->SetupAttachment(BoxMesh);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;
	
	CamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CamComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CamComp->bUsePawnControlRotation = false;

	MovementComponent = CreateDefaultSubobject<UNPS_MovementComponent>(TEXT("MovementComp"));
	if (MovementComponent)
	{
		MovementComponent->UpdatedComponent = PhysRootComp;
		MovementComponent->UpdatedPrimitive = PhysRootComp;
		MovementComponent->ForSmoothVisualComponent = ForSmoothingVisualComp;
	}

 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ANPS_Pawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ANPS_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UPrimitiveComponent* ANPS_Pawn::GetPhysRootComp() const
{
	return PhysRootComp;
}

USceneComponent* ANPS_Pawn::GetForSmoothingVisualComp() const
{
	return ForSmoothingVisualComp;
}

