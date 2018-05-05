// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "INetPhysSync.h"
#include "FAutoRegisterINetPhysSyncTick.h"
#include <GameFramework/PawnMovementComponent.h>
#include "Interfaces/NetworkPredictionInterface.h"
#include "UNPS_MovementComponent.generated.h"

struct FSavedInput;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETPHYSSYNC_API UNPS_MovementComponent : public UPawnMovementComponent, 
	public INetPhysSync, public INetworkPredictionInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNPS_MovementComponent();
	
	UPROPERTY(Transient)
	class USceneComponent* ForSmoothVisualComponent;

	virtual bool IsComponentDataValid() const;

#pragma region INetPhysSync
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

	virtual bool TryGetNewestUnprocessedServerTick(uint32& OutServerTick) const override;

	virtual bool TryGetReplayIndex(uint32& OutTickIndex) const override;

	virtual bool TryGetNewSyncTick(FTickSyncPoint& OutNewSyncPoint) const override;

	virtual bool IsLocalPlayerControlPawn() const override;

	virtual void OnReadReplication(const FOnReadReplicationParam& ReadReplicationParam) override;
#pragma endregion INetPhysSync


#pragma region INetworkPredictionInterface
	//--------------------------------
	// Server hooks
	//--------------------------------

	/** (Server) Call by UNetDriver from server. */
	virtual void SendClientAdjustment();

	/** (Server) Call by PlayerController. Need to update FNetworkPredictionData_Server.ServerTimestamp everytime replication is received.*/
	virtual void ForcePositionUpdate(float DeltaTime);

	//--------------------------------
	// Client hooks
	//--------------------------------

	
	virtual void SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation, const FVector& NewLocation, const FQuat& NewRotation);

	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const;

	virtual class FNetworkPredictionData_Server* GetPredictionData_Server() const;

	virtual bool HasPredictionData_Client() const;

	virtual bool HasPredictionData_Server() const;

	/* This actually call by ACharacter but mine is not character so need to manually call this.*/
	virtual void ResetPredictionData_Client();

	virtual void ResetPredictionData_Server();

#pragma endregion INetworkPredictionInterface

protected:

	UPROPERTY(Transient, DuplicateTransient)
	class ANPS_PawnBase* NPS_PawnOwner;

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

	virtual void SimulatedInput(const FSavedInput& SavedInput);

private:
	class FNPS_ClientPawnPrediction* ClientPawnPrediction;

	class FNPS_ServerPawnPrediction* ServerPawnPrediction;

	FAutoRegisterINetPhysSyncTick AutoRegisterTick;

	UPROPERTY(VisibleAnywhere, Category=Movement)
	FVector MoveSpeedVec;

	UPROPERTY(VisibleAnywhere, Category=Movement)
	float MoveSpeed;

	UPROPERTY(VisibleAnywhere, Category = Movement)
	float InvMoveSpeed;
};
