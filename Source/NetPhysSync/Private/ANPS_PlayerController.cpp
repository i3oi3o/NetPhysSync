// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "ANPS_PlayerController.h"
#include "GameFramework/GameNetworkManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "ConstCollection/InputNameCollection.h"
#include "Interfaces/NetworkPredictionInterface.h"
#include "Engine/World.h"
#include "ANPS_PawnBase.h"
#include "UNPS_MovementComponent.h"


ANPS_PlayerController::ANPS_PlayerController(const FObjectInitializer& ObjectInitializer)
{
	TurnRate = 30;
}

void ANPS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent != nullptr);

	ENetMode NetMode = GetNetMode();

	if (
		(
			NetMode == ENetMode::NM_Client &&
			Role == ENetRole::ROLE_AutonomousProxy
			)
		||
		NetMode == ENetMode::NM_ListenServer ||
		NetMode == ENetMode::NM_Standalone
		)
	{
		InputComponent->BindAxis(InputNameCollection::MOVE_FORWARD_AXIS_NAME, this,
			&ANPS_PlayerController::MoveForward);
		InputComponent->BindAxis(InputNameCollection::MOVE_RIGHT_AXIS_NAME, this,
			&ANPS_PlayerController::MoveRight);
		InputComponent->BindAxis(InputNameCollection::TURN_RIGHT_AXIS_NAME, this,
			&ANPS_PlayerController::TurnRight);
		InputComponent->BindAxis(InputNameCollection::TURN_UP_AXIS_NAME, this,
			&ANPS_PlayerController::TurnUp);
		InputComponent->BindAxis(InputNameCollection::TURN_RATE_RIGHT_AXIS_NAME, this,
			&ANPS_PlayerController::TurnRateRight);
		InputComponent->BindAxis(InputNameCollection::TURN_RATE_UP_AXIS_NAME, this,
			&ANPS_PlayerController::TurnRateUp);
	}
}

void ANPS_PlayerController::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	// Code is copy paste from PlayerController.cpp
	if (
			(GetRemoteRole() == ROLE_AutonomousProxy) && 
			!IsNetMode(NM_Client) && 
			!IsLocalPlayerController()
	   )
	{
		
		if (
				GetPawn() != nullptr && !GetPawn()->IsPendingKill() &&
				GetPawn()->GetRemoteRole() == ROLE_AutonomousProxy &&
				GetPawn()->GetIsReplicated()
		   )
		{
			UMovementComponent* PawnMovement = GetPawn()->GetMovementComponent();
			INetworkPredictionInterface* INet = Cast<INetworkPredictionInterface>(PawnMovement);
			FNetworkPredictionData_Server* ServerPrediction = nullptr;

			if (INet != nullptr && INet->HasPredictionData_Server())
			{
				ServerPrediction = INet->GetPredictionData_Server();
			}
			

			if (ServerPrediction != nullptr)
			{
				UWorld* World = GetWorld();
				if (ServerPrediction->ServerTimeStamp != 0.0f)
				{
					const float TimeSinceUpdate = World->GetTimeSeconds() 
						- ServerPrediction->ServerTimeStamp;
					const float PawnTimeSinceUpdate = TimeSinceUpdate 
						* GetPawn()->CustomTimeDilation;
					float Threshold = AGameNetworkManager::StaticClass()
						->GetDefaultObject<AGameNetworkManager>()
						->MAXCLIENTUPDATEINTERVAL 
						* GetPawn()->GetActorTimeDilation();

					if (PawnTimeSinceUpdate > Threshold)
					{
						ServerPrediction->ServerTimeStamp = World->GetTimeSeconds();
						ServerPrediction = nullptr;

						INet->ForcePositionUpdate(PawnTimeSinceUpdate);
					}
				}
				else
				{
					ServerPrediction->ServerTimeStamp = World->GetTimeSeconds();
				}
			}
		}
	}
}

void ANPS_PlayerController::MoveForward(float InputValue)
{
	if (CanAddMoveInput())
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		GetPawn()->AddMovementInput(Direction, InputValue);
	}
}

void ANPS_PlayerController::MoveRight(float InputValue)
{
	if (CanAddMoveInput())
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		GetPawn()->AddMovementInput(Direction, InputValue);
	}
}

void ANPS_PlayerController::TurnUp(float InputValue)
{
	if (CanAddTurnInput())
	{
		AddPitchInput(InputValue);
	}
}

void ANPS_PlayerController::TurnRight(float InputValue)
{
	if (CanAddTurnInput())
	{
		AddYawInput(InputValue);
	}
}

void ANPS_PlayerController::TurnRateUp(float InputValue)
{
	if (CanAddTurnInput())
	{
		 AddPitchInput(InputValue * TurnRate * GetPawn()->GetWorld()->GetDeltaSeconds());
	}
}

void ANPS_PlayerController::TurnRateRight(float InputValue)
{
	if (CanAddTurnInput())
	{
		AddYawInput(InputValue * TurnRate * GetPawn()->GetWorld()->GetDeltaSeconds());
	}
}

bool ANPS_PlayerController::CanAddMoveInput()
{
	APawn* Pawn = GetPawn();
	ANPS_PawnBase* NPSPawn = Cast<ANPS_PawnBase>(Pawn);
	if (NPSPawn != nullptr && Pawn->GetWorld() != nullptr)
	{
		UNPS_MovementComponent* MovementComponent = Cast<UNPS_MovementComponent>(NPSPawn->GetMovementComponent());
		return MovementComponent != nullptr && 
			MovementComponent->IsStartPhysicYet();
	}

	return false;
}

bool ANPS_PlayerController::CanAddTurnInput()
{
	return GetPawn() != nullptr && GetPawn()->GetWorld() != nullptr;
}
