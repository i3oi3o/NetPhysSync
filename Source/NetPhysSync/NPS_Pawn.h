// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NPS_Pawn.generated.h"

UCLASS()
class NETPHYSSYNC_API ANPS_Pawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ANPS_Pawn();

protected:
	UPROPERTY(VisibleAnywhere, Category = Physics)
	UPrimitiveComponent* PhysRootComp;
	UPROPERTY(VisibleAnywhere, Category = Visual)
	UStaticMeshComponent* VisualComp;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* CamComp;
	UPROPERTY(VisibleAnywhere, Category = Visual)
	USceneComponent* ForSmoothingVisualComp;
	UPROPERTY(VisibleAnywhere, Category = MovementComponent)
	class UNPS_MovementComponent* MovementComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPrimitiveComponent* GetPhysRootComp() const;
	USceneComponent* GetForSmoothingVisualComp() const;

};
