// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UNPSNetSetting.generated.h"

/**
 * 
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "NPSNetSetting"))
class NETPHYSSYNC_API UNPSNetSetting : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UNPSNetSetting(const FObjectInitializer& OI);

	UPROPERTY(config, EditAnywhere, Category=NetworkSynchronization)
	float SupportRTTInMS;
	
	UPROPERTY(config, EditAnywhere, Category = NetworkSynchronization)
	int32 JitterWaitPhysTick;

	static UNPSNetSetting* Get()
	{
		return CastChecked<UNPSNetSetting>(UNPSNetSetting::StaticClass()->GetDefaultObject());
	}
	
};
