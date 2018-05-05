// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FAutoProxyCorrect.h"

FAutoProxyCorrect::FAutoProxyCorrect()
{
}

FAutoProxyCorrect::FAutoProxyCorrect
(
	const FReplicatedRigidBodyState& ReplicatedRigidBodyStateParam, 
	uint32 ServerTickParam
)
	: ReplicatedRigidBodyState(ReplicatedRigidBodyStateParam)
	, ServerTick(ServerTickParam)
{

}

FAutoProxyCorrect::~FAutoProxyCorrect()
{
}
