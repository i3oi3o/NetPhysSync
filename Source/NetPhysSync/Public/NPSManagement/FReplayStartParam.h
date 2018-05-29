// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "FOnNewSyncPointInfo.h"

class FPhysScene;
enum EPhysicsSceneType;

/**
 * 
 */
struct NETPHYSSYNC_API FReplayStartParam
{
public:
	FReplayStartParam
	(
		FPhysScene* const PhysSceneParam,
		const EPhysicsSceneType SceneTypeParam,
		const FOnNewSyncPointInfo NewSyncPointInfoParam,
		const uint32 StartReplayTickIndexParam,
		const uint32 CurrentTickIndexParam
	);
	~FReplayStartParam();

	/*
	* This can happen if client run slower than server.
	*/
	bool IsReplayIntoFuture() const;

	/*
	* ReplayNum is zero if replay into future.
	*/
	int32 GetReplayNum() const;

	FPhysScene* const PhysScene;
	const EPhysicsSceneType SceneType;
	const FOnNewSyncPointInfo NewSyncPointInfo;
	const uint32 StartReplayTickIndex;
	const uint32 CurrentTickIndex;
};
