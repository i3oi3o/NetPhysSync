// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FAutoProxySyncCorrect.h"
#include "FNPS_StaticHelperFunction.h"

FAutoProxySyncCorrect::FAutoProxySyncCorrect()
	: SyncClientTick(0)
	, SyncServerTick(0)
	, ReplicateState()
	, OffsetClientTickToGetLastProcessedInputTick(TNumericLimits<int32>::Max())
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

FAutoProxySyncCorrect::FAutoProxySyncCorrect
(
	const FReplicatedRigidBodyState& ReplicateStateParam, 
	uint32 SyncClientTickParam, 
	uint32 SyncServerTickParam, 
	uint32 LastProcessedClientInputTick
)
	: SyncClientTick(SyncClientTickParam)
	, SyncServerTick(SyncServerTickParam)
	, ReplicateState(ReplicateStateParam)
{
	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		SyncClientTick,
		LastProcessedClientInputTick,
		OffsetClientTickToGetLastProcessedInputTick
	);
}

FAutoProxySyncCorrect::~FAutoProxySyncCorrect()
{
}

bool FAutoProxySyncCorrect::TryGetLastProcessedClientInputTick(uint32& OutTick) const
{
	OutTick = SyncClientTick + OffsetClientTickToGetLastProcessedInputTick;
	return OffsetClientTickToGetLastProcessedInputTick <=
		FNPS_StaticHelperFunction::GetPositiveInclusiveThresholdForOldTick();
}
