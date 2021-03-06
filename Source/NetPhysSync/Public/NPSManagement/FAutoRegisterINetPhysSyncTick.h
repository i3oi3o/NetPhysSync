// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include <Engine/EngineBaseTypes.h>
#include "FAutoRegisterINetPhysSyncTick.generated.h"

class INetPhysSync;

/**
 * Investigate if we need this. A pawn probably not exist until Map is loaded.
 * When BeginPlay is called on client, GameState and PlayerController probably exist already.
 */
USTRUCT()
struct NETPHYSSYNC_API FAutoRegisterINetPhysSyncTick : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

public:
	FAutoRegisterINetPhysSyncTick();
	virtual ~FAutoRegisterINetPhysSyncTick();
	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	virtual FString DiagnosticMessage() override;
	virtual void StartAutoRegister(TScriptInterface<class INetPhysSync> ToRegister);
	virtual void StopAutoRegister();

	FORCEINLINE TScriptInterface<INetPhysSync> GetRegistee() const
	{
		return ToRegister;
	}

private:
	TScriptInterface<INetPhysSync> ToRegister;
};


template<>
struct TStructOpsTypeTraits<FAutoRegisterINetPhysSyncTick> : 
	public TStructOpsTypeTraitsBase2<FAutoRegisterINetPhysSyncTick>
{
	enum
	{
		WithCopy = false
	};
};