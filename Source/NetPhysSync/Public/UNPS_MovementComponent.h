// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "INetPhysSync.h"
#include "FAutoRegisterINetPhysSyncTick.h"
#include <GameFramework/PawnMovementComponent.h>
#include "Interfaces/NetworkPredictionInterface.h"
#include "FAutoProxyCorrect.h"
#include "UNPS_MovementComponent.generated.h"


struct FSavedInput;
struct FAutonomousProxyInput;
struct FAutoProxySyncCorrect;
struct FAutoProxyCorrect;
class FNPS_ClientPawnPrediction;
class FNPS_ServerPawnPrediction;
namespace physx
{
	class PxRigidDynamic;
}


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

// -------------------- Start INetPhysSync -------------------
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

	virtual void OnFinishReadReplication(const FOnFinishReadAllReplicationParam& FinishReadRepParam) override;
// ----------------- End INetPhysSync -----------------


// -------------- Start INetworkPredictionInterface -------------------
	//--------------------------------
	// Server hooks
	//--------------------------------

	/** (Server) Call by UNetDriver from server. */
	virtual void SendClientAdjustment();

	/** (Server) Call by PlayerController. Need to update FNetworkPredictionData_Server.ServerTimestamp every time replication is received.*/
	virtual void ForcePositionUpdate(float DeltaTimeSinceLastCorrection);

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

//	--------------End INetworkPredictionInterface------------------ -

// -------------- Start RPC Function ------------------
	/*
	* This component doesn't enable replication flag.
	* Actor is responsible for replication.
	* All these function just route call to Actor's RPC.
	* Received RPC call on client/server, it will call function implementation in this component.
	*/


	void Server_UpdateAutonomousInput(FAutonomousProxyInput&& AutonomousProxyInpit);
	void Server_UpdateAutonomousInput_Imlementation(const FAutonomousProxyInput& AutonomousProxyInpit);

	void Client_CorrectStateWithSyncTick(const FAutoProxySyncCorrect& AutoProxySyncCorrect);
	void Client_CorrectStateWithSyncTick_Implementation(const FAutoProxySyncCorrect& AutoProxySyncCorrect);

	void Client_CorrectState(const FAutoProxyCorrect& Correction);
	void Client_CorrectState_Implementation(const FAutoProxyCorrect& Correction);

	

// ------------- End RPC Function ------------------

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

	bool bServerHasAutoProxyPendingCorrection;

	bool bClientHasNewSyncPoint;

	bool bClientHasRecievedNewServerTick;

	bool bClientHasReceivedServerTick;

	uint32 ClientReceivedServerTick;

	FTickSyncPoint ClientReceiveTickSyncPoint;

//----------- Begin Receive Force update on client ----------
	/*
	* This is cache and used later in INetPhysSync::OnReadReplication(Args...)
	*/

	bool bClientHasAutoCorrectWithoutSyncTick;

	FAutoProxyCorrect ClientAutoProxyCorrectWithoutSyncTick;
//----------- End Receive Force update on client -------------

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void SimulatedInput(const FSavedInput& SavedInput);

	FNPS_ClientPawnPrediction* GetPredictionData_ClientNPSPawn() const;

	FNPS_ServerPawnPrediction* GetPredictionData_ServerNPSPawn() const;

	physx::PxRigidDynamic* GetUpdatedRigidDynamic();

	bool IsLateSyncClientTick
	(
		const FNPS_ClientPawnPrediction* ClientPrediction,
		const FAutoProxySyncCorrect& SyncCorrect
	) const;

	/**
	 * Handle out of order package from server
	 */
	bool IsReceivedServerTickTooOld(uint32 ServerTick);

	void ResetClientCachReceiveDataFlag();

	void DrawDebugRigidBody(const FReplicatedRigidBodyState& DrawPos, const FColor& Color);

private:
	FNPS_ClientPawnPrediction* ClientPawnPrediction;

	FNPS_ServerPawnPrediction* ServerPawnPrediction;

	FAutoRegisterINetPhysSyncTick AutoRegisterTick;

	UPROPERTY(VisibleAnywhere, Category=Movement)
	FVector MoveSpeedVec;

	UPROPERTY(VisibleAnywhere, Category=Movement)
	float MoveSpeed;

	UPROPERTY(VisibleAnywhere, Category = Movement)
	float InvMoveSpeed;
};
