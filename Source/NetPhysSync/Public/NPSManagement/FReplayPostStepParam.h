// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"


class FPhysScene;
enum EPhysicsSceneType;

/**
 * 
 */
struct NETPHYSSYNC_API FReplayPostStepParam
{
public:
	FReplayPostStepParam
	(
		FPhysScene* const PhysSceneParam,
		const EPhysicsSceneType SceneTypeParam,
		const float StepDeltaTimeParam,
		const uint32 ReplayTickIndexParam
	);
	~FReplayPostStepParam();

	FPhysScene* const PhysScene;
	const EPhysicsSceneType SceneType;
	const float StepDeltaTime;
	const uint32 ReplayTickIndex;
};
