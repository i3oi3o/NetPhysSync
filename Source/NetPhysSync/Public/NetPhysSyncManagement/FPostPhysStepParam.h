// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct NETPHYSSYNC_API FPostPhysStepParam
{
public:
	FPostPhysStepParam(class FPhysScene* const PhysScene, 
		const uint32 SceneType, float StepDeltaTime,
		const uint32 LocalNetPhysTicks);
	~FPostPhysStepParam();

public:
	class FPhysScene* const PhysScene;
	const enum EPhysicsSceneType SceneType;
	const float StepDeltaTime;
	const uint32 LocalNetPhysTicks;
};
