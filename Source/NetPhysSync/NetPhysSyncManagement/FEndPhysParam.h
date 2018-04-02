// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct NETPHYSSYNC_API FEndPhysParam
{

public:
	FEndPhysParam(uint32 SceneType, float StartDeltaTime, uint32 LocalNetPhysTicks);
	~FEndPhysParam();

public:
	const enum EPhysicsSceneType SceneType;
	const float StartDeltaTime;
	const uint32 LocalNetPhysTicks;
};
