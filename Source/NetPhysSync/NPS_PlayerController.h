// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NPS_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NETPHYSSYNC_API ANPS_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
	public:
		ANPS_PlayerController(const FObjectInitializer& ObjectInitializer);

	public:
		UPROPERTY(EditAnywhere)
		float TurnRate;

	protected:
		virtual void SetupInputComponent() override;
	
	private:
		void MoveForward(float InputValue);
		void MoveRight(float InputValue);
		void TurnUp(float InputValue);
		void TurnRight(float InputValue);
		void TurnRateUp(float InputValue);
		void TurnRateRight(float InputValue);
};
