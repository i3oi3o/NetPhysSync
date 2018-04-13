// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "NPS_DynamicBoxActor.h"
#include "Components/BoxComponent.h"




ANPS_DynamicBoxActor::ANPS_DynamicBoxActor(const FObjectInitializer& OI)
	: Super(OI.SetDefaultSubobjectClass<UBoxComponent>(TEXT("RigidBody")))
{
	UBoxComponent* BoxComponent = static_cast<UBoxComponent*>(GetUpdatedPrimitive());
	BoxComponent->bDynamicObstacle = true;

}
