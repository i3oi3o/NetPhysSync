// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ANPSGameState.generated.h"

USTRUCT()
struct FNPSGameStatePostPhysicsTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

	class FNetPhysSyncManager* Target;


private:
	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

};

template<>
struct TStructOpsTypeTraits<FNPSGameStatePostPhysicsTickFunction>
	: public TStructOpsTypeTraitsBase2<FNPSGameStatePostPhysicsTickFunction>
{
	enum 
	{
		WithCopy = false
	};
};

/**
 * 
 */
UCLASS()
class NETPHYSSYNC_API ANPSGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ANPSGameState();
	void RegisterINetPhysSync(TScriptInterface<class INetPhysSync> ToRegister);
	void UnregisterINetPhysSync(TScriptInterface<class INetPhysSync> ToUnregister);
	virtual void RegisterActorTickFunctions(bool bRegister) override;
	virtual void BeginDestroy() override;

protected:
	virtual void BeginPlay() override;

private:
	class FNetPhysSyncManager* NetPhysSyncManager;
	class FNetPhysSyncManager* GetOrCreateNetPhysSyncManager();
	FNPSGameStatePostPhysicsTickFunction PostPhysicTickFunction;
	bool bBeginDestroy;
	
};
