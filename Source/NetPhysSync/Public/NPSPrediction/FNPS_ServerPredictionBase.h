// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/NetworkPredictionInterface.h"

/**
 * 
 */
class NETPHYSSYNC_API FNPS_ServerPredictionBase : public FNetworkPredictionData_Server, protected FNoncopyable
{
	typedef FNetworkPredictionData_Server Super;
public:
	FNPS_ServerPredictionBase();
	virtual ~FNPS_ServerPredictionBase();
};
