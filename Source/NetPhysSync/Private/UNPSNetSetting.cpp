// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "UNPSNetSetting.h"




UNPSNetSetting::UNPSNetSetting(const FObjectInitializer& OI)
	: Super(OI)
	, SupportRTTInMS(200.0f)
	, JitterWaitPhysTick(3)
{

}
