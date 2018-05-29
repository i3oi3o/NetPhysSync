// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FReplayStartParam.h"
#include "PhysicsPublic.h"
#include "FNPS_StaticHelperFunction.h"

FReplayStartParam::FReplayStartParam
(
	FPhysScene* const PhysSceneParam,
	const EPhysicsSceneType SceneTypeParam,
	const FOnNewSyncPointInfo NewSyncPointInfoParam,
	const uint32 StartReplayTickIndexParam,
	const uint32 CurrentTickIndexParam
)
	: PhysScene(PhysSceneParam)
	, SceneType(SceneTypeParam)
	, NewSyncPointInfo(NewSyncPointInfoParam)
	, StartReplayTickIndex(StartReplayTickIndexParam)
	, CurrentTickIndex(CurrentTickIndexParam)
{
}

FReplayStartParam::~FReplayStartParam()
{
}

bool FReplayStartParam::IsReplayIntoFuture() const
{
	int32 ReplayCount = 0;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		CurrentTickIndex, 
		StartReplayTickIndex,
		ReplayCount
	);

	return ReplayCount < 0;
}

int32 FReplayStartParam::GetReplayNum() const
{
	int32 ReplayCount = 0;
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		CurrentTickIndex,
		StartReplayTickIndex,
		ReplayCount
	);

	return FMath::Max(ReplayCount, 0);
}

