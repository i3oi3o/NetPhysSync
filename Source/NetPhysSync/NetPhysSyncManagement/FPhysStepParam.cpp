// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FPhysStepParam.h"
#include <PhysicsPublic.h>



FPhysStepParam::FPhysStepParam(FPhysScene* const PhysScene, const uint32 SceneType, float StepDeltaTime)
	: PhysScene(PhysScene)
	, SceneType(static_cast<EPhysicsSceneType>(SceneType))
	, StepDeltaTime(StepDeltaTime)
{

}

FPhysStepParam::~FPhysStepParam()
{
}
