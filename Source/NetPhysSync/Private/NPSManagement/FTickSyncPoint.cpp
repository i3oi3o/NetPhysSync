// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FTickSyncPoint.h"
#include "FNPS_StaticHelperFunction.h"

FTickSyncPoint::FTickSyncPoint()
	: ClientTickSyncPoint(0)
	, ServerTickSyncPoint(0)
	, bIsValid(false)
{

}

FTickSyncPoint::FTickSyncPoint
(
	const uint32 ClientTickSyncPointParam,
	const uint32 ServerTickSyncPointParam
)
	: ClientTickSyncPoint(ClientTickSyncPointParam)
	, ServerTickSyncPoint(ServerTickSyncPointParam)
	, bIsValid(true)
{
}

FTickSyncPoint::~FTickSyncPoint()
{
}

uint32 FTickSyncPoint::ServerTick2ClientTick(uint32 ServerTick) const
{
	int32 OutDiff;

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		ServerTickSyncPoint,
		ServerTick,
		OutDiff
	);

	return ClientTickSyncPoint + OutDiff;
}

uint32 FTickSyncPoint::ClientTick2ServerTick(uint32 ClientTick) const
{

	int32 OutDiff;

	FNPS_StaticHelperFunction::CalculateBufferArrayIndex
	(
		ClientTickSyncPoint,
		ClientTick,
		OutDiff
	);

	return ServerTickSyncPoint + OutDiff;
}
