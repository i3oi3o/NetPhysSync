// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FNPS_ServerPawnPrediction.h"
#include "UNPSNetSetting.h"



FNPS_ServerPawnPrediction::FNPS_ServerPawnPrediction
(
	const UNPSNetSetting* const NetSettingParam
)
	: Super()
	, InputBuffer(NPS_BUFFER_SIZE)
	, NetSetting(NetSettingParam)
	, InputStartServerTickIndex(0)
	, InputStartClientTickIndex(0)
	, LastProcessedClientTickIndex(0)
	, bHasLastProcessedInputClientTickIndex(false)
	, SyncClientTickIndex(0)
	, bHasSyncClientTickIndex(false)
	, LastProcessedServerTickIndex(0)
	, bHasLastProcessedServerTickIndex(false)
{

}

FNPS_ServerPawnPrediction::~FNPS_ServerPawnPrediction()
{
}

void FNPS_ServerPawnPrediction::UpdateInputBuffer
	(
		const class FAutonomousProxyInput& AutonomousProxyInput, 
		uint32 ReceiveServerTickIndex
	)
{
	unimplemented();
}

const FSavedInput& FNPS_ServerPawnPrediction::ProcessServerTick(uint32 ServerTickIndex)
{
	unimplemented();
	return FSavedInput::EmptyInput;
}
