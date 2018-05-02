// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

enum EPhysicsSceneType;

/**
 * 
 */
struct NETPHYSSYNC_API FReplayEndParam
{
public:
	FReplayEndParam
	(
		const EPhysicsSceneType SceneTypeParam, 
		const uint32 CurrentTickIndexParam
	);
	~FReplayEndParam();

	const EPhysicsSceneType SceneType;
	const uint32 CurrentTickIndex;
};
