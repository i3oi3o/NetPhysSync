// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FEndPhysParam.h"



FEndPhysParam::FEndPhysParam(uint32 SceneType, 
	float StartDeltaTime, uint32 LocalNetPhysTicks)
	: SceneType(static_cast<EPhysicsSceneType>(SceneType)),
	StartDeltaTime(StartDeltaTime),
	LocalNetPhysTicks(LocalNetPhysTicks)
{

}

FEndPhysParam::~FEndPhysParam()
{
}
