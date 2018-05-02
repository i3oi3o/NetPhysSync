// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"


/**
 * 
 */
class NETPHYSSYNC_API FIsTickEnableParam
{
public:
	FIsTickEnableParam(const uint32 SceneTypeParam);
	~FIsTickEnableParam();

	const EPhysicsSceneType SceneType;
};
