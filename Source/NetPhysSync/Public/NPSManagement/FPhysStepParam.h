// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

enum EPhysicsSceneType;

/**
 * 
 */
struct NETPHYSSYNC_API FPhysStepParam
{
public:
	FPhysStepParam
	(
		class FPhysScene* const PhysSceneParam, 
		const EPhysicsSceneType SceneTypeParam,
		float StepDeltaTimeParam, 
		uint32 LocalPhysTickIndexParam
	);
	~FPhysStepParam();

public:
	class FPhysScene* const PhysScene;
	const EPhysicsSceneType SceneType;
	const float StepDeltaTime;
	const uint32 LocalPhysTickIndex;
};
