// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

enum EPhysicsSceneType;

/**
 * 
 */
struct NETPHYSSYNC_API FEndPhysParam
{

public:
	FEndPhysParam
	(
		const EPhysicsSceneType SceneTypeParam, 
		const float StartDeltaTimeParam,
		const uint32 NextPhysTickIndexParam
	);
	~FEndPhysParam();

public:
	const EPhysicsSceneType SceneType;
	const float StartDeltaTime;
	const uint32 NextPhysTickIndex;
};
