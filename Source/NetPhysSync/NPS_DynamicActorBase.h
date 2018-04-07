// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetPhysSyncManagement/INetPhysSync.h"
#include "NetPhysSyncManagement/FAutoRegisterINetPhysSyncTick.h"
#include "NPS_DynamicActorBase.generated.h"

class UPrimitiveComponent;
class USceneComponent;

UCLASS(abstract)
class NETPHYSSYNC_API ANPS_DynamicActorBase : public AActor, public INetPhysSync
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANPS_DynamicActorBase(const FObjectInitializer& OI);



	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPrimitiveComponent* GetUpdatedPrimitive() const;

	virtual bool IsTickEnabled() const  override;

	virtual void TickStartPhysic(const FStartPhysParam& param) override;

	virtual void TickPhysStep(const FPhysStepParam& param) override;

	virtual void TickPostPhysStep(const FPostPhysStepParam& param) override;

	virtual void TickEndPhysic(const FEndPhysParam& param) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnUpdatedPrimitiveGetHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void BeginDestroy() override;
	

private:
	bool bRecentlyGetHit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test", meta = (AllowPrivateAccess = "true"))
	FVector TestSleepVelocity;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UPrimitiveComponent* UpdatedPrimitive;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Test", meta = (AllowPrivateAccess = "true"))
	USceneComponent* ForSmoothingVisual;

	FAutoRegisterINetPhysSyncTick AutoRegisterINetPhysSync;

	

};
