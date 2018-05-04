// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FOnReadReplicationParam.h"



FOnReadReplicationParam::FOnReadReplicationParam
(
	const FOnNewSyncPointInfo NewSyncPointInfoParam
)
	: NewSyncPointInfo(NewSyncPointInfoParam)
{

}

FOnReadReplicationParam::~FOnReadReplicationParam()
{
}
