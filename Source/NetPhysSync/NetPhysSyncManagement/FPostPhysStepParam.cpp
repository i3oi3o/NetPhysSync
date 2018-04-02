// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FPostPhysStepParam.h"
#include <PhysicsPublic.h>



FPostPhysStepParam::FPostPhysStepParam(FPhysScene* const PhysScene, 
	const uint32 SceneType, float StepDeltaTime, uint32 LocalNetPhysTicks)
	: PhysScene(PhysScene),
	SceneType(static_cast<EPhysicsSceneType>(SceneType)),
	StepDeltaTime(StepDeltaTime),
	LocalNetPhysTicks(LocalNetPhysTicks)
{

}



FPostPhysStepParam::~FPostPhysStepParam()
{
}
