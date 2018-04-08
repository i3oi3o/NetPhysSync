// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "NPS_DynamicActorBase.h"
#include "Components/PrimitiveComponent.h"
#include "PxRigidDynamic.h"
#include "PhysXPublic.h"
#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "NPSGameState.h"

using namespace physx;

// Sets default values
ANPS_DynamicActorBase::ANPS_DynamicActorBase(const FObjectInitializer& OI)
	: Super(OI),
	bRecentlyGetHit(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	UpdatedPrimitive = OI.CreateAbstractDefaultSubobject<UPrimitiveComponent>(this, TEXT("RigidBody"));
	
	if (UpdatedPrimitive != nullptr)
	{
		UpdatedPrimitive->SetSimulatePhysics(true);
		UpdatedPrimitive->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
		UpdatedPrimitive->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		UpdatedPrimitive->bShouldUpdatePhysicsVolume = true;
		UpdatedPrimitive->bCheckAsyncSceneOnMove = false;
		UpdatedPrimitive->SetNotifyRigidBodyCollision(true);
		UpdatedPrimitive->OnComponentHit.AddDynamic(this, &ANPS_DynamicActorBase::OnUpdatedPrimitiveGetHit);
	}

	RootComponent = UpdatedPrimitive;
	

	ForSmoothingVisual = OI.CreateDefaultSubobject<USceneComponent>(this, TEXT("ForSmoothingVisual"));
	ForSmoothingVisual->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ANPS_DynamicActorBase::BeginPlay()
{
	Super::BeginPlay();
	if (UpdatedPrimitive != nullptr && 
		UpdatedPrimitive->GetBodyInstance()->IsDynamic())
	{
		AutoRegisterINetPhysSync.StartAutoRegister(this);
		
		//UE_LOG(LogTemp, Log, TEXT("Listen collision."));
	}
}


void ANPS_DynamicActorBase::OnUpdatedPrimitiveGetHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherComp->GetBodyInstance()->IsDynamic())
	{
		bRecentlyGetHit = true;
		UE_LOG(LogTemp, Log, TEXT("GetHit by %s"), *OtherActor->GetName());
		
	}
}

void ANPS_DynamicActorBase::BeginDestroy()
{
	Super::BeginDestroy();
	AutoRegisterINetPhysSync.StopAutoRegister();

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

bool ANPS_DynamicActorBase::IsTickEnabled() const
{
	return !IsPendingKill() && 
		UpdatedPrimitive != nullptr && 
		!UpdatedPrimitive->IsPendingKill();
}

void ANPS_DynamicActorBase::TickStartPhysic(const FStartPhysParam& param) 
{
	
}

void ANPS_DynamicActorBase::TickPhysStep(const FPhysStepParam& param) 
{

}

void ANPS_DynamicActorBase::TickPostPhysStep(const FPostPhysStepParam& param) 
{
	
}

void ANPS_DynamicActorBase::TickEndPhysic(const FEndPhysParam& param) 
{
	
}

// Called every frame
void ANPS_DynamicActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UPrimitiveComponent* ANPS_DynamicActorBase::GetUpdatedPrimitive() const
{
	return UpdatedPrimitive;
}
