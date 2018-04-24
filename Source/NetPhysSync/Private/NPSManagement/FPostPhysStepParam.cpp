// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FPostPhysStepParam.h"
#include <PhysicsPublic.h>



FPostPhysStepParam::FPostPhysStepParam
(
	FPhysScene* const PhysSceneParam, 
	const uint32 SceneTypeParam, float StepDeltaTimeParam, 
	uint32 LocalPhysTickIndexParam
)
	: PhysScene(PhysSceneParam)
	, SceneType(static_cast<EPhysicsSceneType>(SceneTypeParam))
	, StepDeltaTime(StepDeltaTimeParam)
	, LocalPhysTickIndex(LocalPhysTickIndexParam)
{

}



FPostPhysStepParam::~FPostPhysStepParam()
{
}