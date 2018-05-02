// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FReplaySubstepParam.h"
#include "PhysicsPublic.h"



FReplaySubstepParam::FReplaySubstepParam
(
	FPhysScene* const PhysSceneParam, 
	const EPhysicsSceneType SceneTypeParam, 
	const float StepDeltaTimeParam, 
	const uint32 ReplayTickIndexParam
)
	: PhysScene(PhysSceneParam)
	, SceneType(SceneTypeParam)
	, StepDeltaTime(StepDeltaTimeParam)
	, ReplayTickIndex(ReplayTickIndexParam)
{

}

FReplaySubstepParam::~FReplaySubstepParam()
{
}
