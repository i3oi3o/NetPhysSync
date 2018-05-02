// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FReplayEndParam.h"
#include "Engine/EngineTypes.h"

FReplayEndParam::FReplayEndParam
(
	const EPhysicsSceneType SceneTypeParam,
	const uint32 CurrentTickIndexParam
)
	: SceneType(SceneTypeParam)
	, CurrentTickIndex(CurrentTickIndexParam)
{
}

FReplayEndParam::~FReplayEndParam()
{
}
