// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FEndPhysParam.h"



FEndPhysParam::FEndPhysParam
(
	uint32 SceneTypeParam, 
	float StartDeltaTimeParam, 
	uint32 LocalPhysTickIndexParam
)
	: SceneType(static_cast<EPhysicsSceneType>(SceneTypeParam))
	, StartDeltaTime(StartDeltaTimeParam)
	, LocalPhysTickIndex(LocalPhysTickIndexParam)
{

}

FEndPhysParam::~FEndPhysParam()
{
}
