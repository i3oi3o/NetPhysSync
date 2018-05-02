// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

enum EPhysicsSceneType;

/**
 * 
 */
class NETPHYSSYNC_API FIsTickEnableParam
{
public:
	FIsTickEnableParam(const EPhysicsSceneType SceneTypeParam);
	~FIsTickEnableParam();

	const EPhysicsSceneType SceneType;
};
