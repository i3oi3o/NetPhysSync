// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "IOnReplayEnd.h"
#include "ANPSGameState.generated.h"

class ANPSGameState;

USTRUCT()
struct FNPSGameStatePostPhysicsTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

	ANPSGameState* Target;


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
class NETPHYSSYNC_API ANPSGameState : public AGameStateBase, 
	public IQueryReceivedPackage, public IOnReplayEnd
{
	GENERATED_BODY()

public:
	ANPSGameState();

	UPROPERTY(ReplicatedUsing=OnRep_ServerTick)
	uint32 RepServerTick;

	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	void RegisterINetPhysSync(TScriptInterface<class INetPhysSync> ToRegister);
	void UnregisterINetPhysSync(TScriptInterface<class INetPhysSync> ToUnregister);
	virtual void OnTickPostPhysic(float DeltaTime);
	virtual void RegisterActorTickFunctions(bool bRegister) override;
	virtual void BeginDestroy() override;
	virtual bool TryGetNewestUnprocessedServerTick(uint32& OutServerTickIndex) const override;
	virtual void OnReplayEnd() override;

	uint32 GetCurrentPhysTickIndex();

	UFUNCTION()
	void OnRep_ServerTick();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	class FNetPhysSyncManager* NetPhysSyncManager;
	class FNetPhysSyncManager* GetOrCreateNetPhysSyncManager();
	FNPSGameStatePostPhysicsTickFunction PostPhysicTickFunction;
	bool bBeginDestroy;
	bool bNewUnprocessedServerTick;
	uint32 CachServerTick;
	
};
