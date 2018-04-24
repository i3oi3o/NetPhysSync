// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct NETPHYSSYNC_API FPostPhysStepParam
{
public:
	FPostPhysStepParam
	(
		class FPhysScene* const PhysSceneParam, 
		const uint32 SceneTypeParam, float StepDeltaTimeParam,
		const uint32 LocalPhysTickIndexParam
	);
	~FPostPhysStepParam();

public:
	class FPhysScene* const PhysScene;
	const enum EPhysicsSceneType SceneType;
	const float StepDeltaTime;
	const uint32 LocalPhysTickIndex;
};
