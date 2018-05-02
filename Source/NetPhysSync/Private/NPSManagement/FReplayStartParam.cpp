// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FReplayStartParam.h"

FReplayStartParam::FReplayStartParam
(
	FPhysScene* const PhysSceneParam,
	const EPhysicsSceneType SceneTypeParam,
	const FOnNewSyncPointInfo NewSyncPointInfoParam,
	const uint32 StartReplayTickIndexParam
)
	: PhysScene(PhysSceneParam)
	, SceneType(SceneTypeParam)
	, NewSyncPointInfo(NewSyncPointInfoParam)
	, StartReplayTickIndex(StartReplayTickIndexParam)
{
}

FReplayStartParam::~FReplayStartParam()
{
}
