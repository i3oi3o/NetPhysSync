// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FStartPhysParam.h"
#include <PhysicsPublic.h>



FStartPhysParam::FStartPhysParam
(
	FPhysScene* PhysSceneParam, 
	uint32 SceneTypeParam, float StartDeltaTimeParam,
	uint32 LocalPhysTickIndexParam
) 
	: PhysScene(PhysSceneParam)
	, SceneType(static_cast<EPhysicsSceneType>(SceneTypeParam))
	, StartDeltaTime(StartDeltaTimeParam)
	, LocalPhysTickIndex(LocalPhysTickIndexParam)
{

}

FStartPhysParam::~FStartPhysParam()
{
}
