// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FStartPhysParam.h"
#include <PhysicsPublic.h>



FStartPhysParam::FStartPhysParam(FPhysScene* PhysScene, 
	uint32 SceneType, float StartDeltaTime) 
	: PhysScene(PhysScene),
	SceneType(static_cast<EPhysicsSceneType>(SceneType)),
	StartDeltaTime(StartDeltaTime)
{

}

FStartPhysParam::~FStartPhysParam()
{
}
