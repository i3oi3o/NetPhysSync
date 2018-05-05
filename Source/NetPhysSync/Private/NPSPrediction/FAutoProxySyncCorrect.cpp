// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FAutoProxySyncCorrect.h"

FAutoProxySyncCorrect::FAutoProxySyncCorrect()
	: SyncClientTick(0)
	, SyncServerTick(0)
	, ReplicateState()
{
}

FAutoProxySyncCorrect::FAutoProxySyncCorrect
(
	const FReplicatedRigidBodyState& ReplicateStateParam, 
	uint32 SyncClientTickParam, 
	uint32 SyncServerTickParam
)
	: SyncClientTick(SyncClientTickParam)
	, SyncServerTick(SyncServerTickParam)
	, ReplicateState(ReplicateStateParam)
{

}

FAutoProxySyncCorrect::~FAutoProxySyncCorrect()
{
}
