// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FOnNewSyncPointInfo.h"
#include "FNPS_StaticHelperFunction.h"

int32 CalculateClientTickShiftAmountForReplayPrediction
(
	const FTickSyncPoint& OldSyncPointParam,
	const FTickSyncPoint& NewSyncPointParam
)
{
	int32 ToReturnShiftAmount = 0;

	uint32 UpdatedOldClientTick = NewSyncPointParam.ServerTick2ClientTick
	(
		OldSyncPointParam.GetServerTickSyncPoint()
	);

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		OldSyncPointParam.GetClientTickSyncPoint(),
		UpdatedOldClientTick,
		ToReturnShiftAmount
	);

	return ToReturnShiftAmount;
}

FOnNewSyncPointInfo::FOnNewSyncPointInfo
(
	const FTickSyncPoint& OldSyncPointParam,
	const FTickSyncPoint& NewSyncPointParam
)
	: OldSyncPoint(OldSyncPointParam)
	, NewSyncPoint(NewSyncPointParam)
	, ShiftClientTickAmountForReplayPrediction
	  (
		  CalculateClientTickShiftAmountForReplayPrediction
		  (
			  OldSyncPointParam, NewSyncPointParam
		  )
	  )
{
}

FOnNewSyncPointInfo::~FOnNewSyncPointInfo()
{
}
