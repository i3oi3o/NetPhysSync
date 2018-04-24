// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct NETPHYSSYNC_API FPhysStepParam
{
public:
	FPhysStepParam(class FPhysScene* const PhysScene, const uint32 SceneType, float StepDeltaTime);
	~FPhysStepParam();

public:
	class FPhysScene* const PhysScene;
	const enum EPhysicsSceneType SceneType;
	const float StepDeltaTime;
};
