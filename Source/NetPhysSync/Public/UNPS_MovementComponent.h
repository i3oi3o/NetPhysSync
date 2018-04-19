// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NetPhysSyncManagement/INetPhysSync.h"
#include "NetPhysSyncManagement/FAutoRegisterINetPhysSyncTick.h"
#include <GameFramework/PawnMovementComponent.h>
#include "UNPS_MovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETPHYSSYNC_API UNPS_MovementComponent : public UPawnMovementComponent, 
	public INetPhysSync
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNPS_MovementComponent();
	
	UPROPERTY(Transient)
	class USceneComponent* ForSmoothVisualComponent;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void TickStartPhysic(const FStartPhysParam& param) override;

	virtual void TickPhysStep(const FPhysStepParam& param) override;

	virtual void TickPostPhysStep(const FPostPhysStepParam& param) override;

	virtual void TickEndPhysic(const FEndPhysParam& param) override;

	virtual void BeginDestroy() override;

	virtual bool IsTickEnabled() const override;

protected:

	UPROPERTY(EditDefaultsOnly, Category=Movement)
	float Speed;

	UPROPERTY(EditDefaultsOnly, Category=Movement)
	float MaxVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category=Movement)
	FVector ApplyForceLocalPos;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float MaxAngularVelocityDegree;

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void SimulatedInput(FVector MoveSpeedVecParam);

private:
	FAutoRegisterINetPhysSyncTick AutoRegisterTick;

	UPROPERTY(VisibleAnywhere, Category=Movement)
	FVector MoveSpeedVec;

	UPROPERTY(VisibleAnywhere, Category=Movement)
	float MoveSpeed;

	UPROPERTY(VisibleAnywhere, Category = Movement)
	float InvMoveSpeed;
};
