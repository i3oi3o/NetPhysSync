// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "ANPS_BoxPawn.h"
#include "Components/BoxComponent.h"
#include <Engine/CollisionProfile.h>
#include <Components/StaticMeshComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include <ConstructorHelpers.h>
#include "UNPS_MovementComponent.h"


// Sets default values
ANPS_BoxPawn::ANPS_BoxPawn(const FObjectInitializer& OI)
	: Super(OI.SetDefaultSubobjectClass<UBoxComponent>(TEXT("RigidBody")))
{
	UBoxComponent* BoxComponent = static_cast<UBoxComponent*>(PhysRootComp);
	BoxComponent->InitBoxExtent(FVector(50.0f));
	BoxComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	BoxComponent->bDynamicObstacle = true;
	BoxComponent->bCheckAsyncSceneOnMove = false;
	BoxComponent->bShouldUpdatePhysicsVolume = true;
	BoxComponent->SetSimulatePhysics(true);
}

