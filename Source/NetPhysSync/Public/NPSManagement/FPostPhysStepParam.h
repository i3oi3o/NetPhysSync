// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

enum EPhysicsSceneType;

/**
 * 
 */
struct NETPHYSSYNC_API FPostPhysStepParam
{
public:
	FPostPhysStepParam
	(
		class FPhysScene* const PhysSceneParam, 
		const EPhysicsSceneType SceneTypeParam, 
		float StepDeltaTimeParam,
		const uint32 LocalPhysTickIndexParam
	);
	~FPostPhysStepParam();

public:
	class FPhysScene* const PhysScene;
	const EPhysicsSceneType SceneType;
	const float StepDeltaTime;
	const uint32 LocalPhysTickIndex;
};
