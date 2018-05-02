// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FReplayPostStepParam.h"
#include "PhysicsPublic.h"

FReplayPostStepParam::FReplayPostStepParam
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

FReplayPostStepParam::~FReplayPostStepParam()
{
}
