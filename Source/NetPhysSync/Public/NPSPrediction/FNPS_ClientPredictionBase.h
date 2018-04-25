// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/NetworkPredictionInterface.h"

/**
 * 
 */
class NETPHYSSYNC_API FNPS_ClientPredictionBase : public FNetworkPredictionData_Client, protected FNoncopyable
{
	typedef FNetworkPredictionData_Client Super;

public:
	FNPS_ClientPredictionBase();
	virtual ~FNPS_ClientPredictionBase();
};
