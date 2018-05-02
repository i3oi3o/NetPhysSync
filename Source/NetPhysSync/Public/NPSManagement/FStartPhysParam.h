// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

class FPhysScene;
enum EPhysicsSceneType;

/**
 * Encapsulate parameters to avoid re-implementation of function signature change by parameter.
 */
struct NETPHYSSYNC_API FStartPhysParam
{
public:
	FStartPhysParam
	(
		FPhysScene* const PhysSceneParam, 
		uint32 SceneTypeParam, float StartDeltaTimeParam,
		uint32 LocalPhysTickIndexParam
	);
	~FStartPhysParam();

public:
	FPhysScene* const PhysScene;
	const EPhysicsSceneType SceneType;
	const float StartDeltaTime;
	const uint32 LocalPhysTickIndex;
};
