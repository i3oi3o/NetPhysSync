// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "INetPhysSync.h"
#include "FAutoRegisterINetPhysSyncTick.h"
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

	virtual bool IsTickEnabled(const FIsTickEnableParam& param) const override;

	virtual void TickReplayStart(const FReplayStartParam& param) override;

	virtual void TickReplaySubstep(const FReplaySubstepParam& param) override;

	virtual void TickReplayPostSubstep(const FReplayPostStepParam& param) override;

	virtual void TickReplayEnd(const FReplayEndParam& param) override;

	virtual void VisualUpdate(const FVisualUpdateParam& param) override;

	virtual bool TryGetReplayIndex(uint32& OutTickIndex) const override;

	virtual bool TryGetNewSyncTick(FTickSyncPoint& OutNewSyncPoint) const override;

	virtual bool IsLocalPlayerControlPawn() const override;

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
