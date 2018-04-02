// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * Encapsulate parameters to avoid re-implementation of function signature change by parameter.
 */
struct NETPHYSSYNC_API FStartPhysParam
{
public:
	FStartPhysParam(class FPhysScene* const PhysScene, 
		const uint32 SceneType, const float StartDeltaTime);
	~FStartPhysParam();

public:
	class FPhysScene* const PhysScene;
	const enum EPhysicsSceneType SceneType;
	const float StartDeltaTime;
};
