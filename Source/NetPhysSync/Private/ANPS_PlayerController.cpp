// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "ANPS_PlayerController.h"
#include "GameFramework/Pawn.h"
#include "ConstCollection/InputNameCollection.h"



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

void ANPS_PlayerController::MoveForward(float InputValue)
{
	APawn* Pawn = GetPawn();
	if (Pawn != nullptr)
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		Pawn->AddMovementInput(Direction, InputValue);
	}
}

void ANPS_PlayerController::MoveRight(float InputValue)
{
	APawn* Pawn = GetPawn();
	if (Pawn != nullptr)
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		Pawn->AddMovementInput(Direction, InputValue);
	}
}

void ANPS_PlayerController::TurnUp(float InputValue)
{
	APawn* Pawn = GetPawn();
	if (Pawn != nullptr)
	{
		AddPitchInput(InputValue);
	}
}

void ANPS_PlayerController::TurnRight(float InputValue)
{
	APawn* Pawn = GetPawn();
	if (Pawn != nullptr)
	{
		AddYawInput(InputValue);
	}
}

void ANPS_PlayerController::TurnRateUp(float InputValue)
{
	APawn* Pawn = GetPawn();
	if (Pawn != nullptr)
	{
		 AddPitchInput(InputValue * TurnRate * Pawn->GetWorld()->GetDeltaSeconds());
	}
}

void ANPS_PlayerController::TurnRateRight(float InputValue)
{
	APawn* Pawn = GetPawn();
	if (Pawn != nullptr)
	{
		AddYawInput(InputValue * TurnRate * Pawn->GetWorld()->GetDeltaSeconds());
	}
}
