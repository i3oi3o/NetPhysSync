// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NPS_PawnBase.h"
#include "NPS_BoxPawn.generated.h"

UCLASS()
class NETPHYSSYNC_API ANPS_BoxPawn : public ANPS_PawnBase
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ANPS_BoxPawn(const FObjectInitializer& OI);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPrimitiveComponent* GetPhysRootComp() const;
	USceneComponent* GetForSmoothingVisualComp() const;

};