// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FIsTickEnableParam.h"

FIsTickEnableParam::FIsTickEnableParam(const uint32 SceneTypeParam)
	: SceneType(static_cast<EPhysicsSceneType>(SceneTypeParam))
{
}

FIsTickEnableParam::~FIsTickEnableParam()
{
}
