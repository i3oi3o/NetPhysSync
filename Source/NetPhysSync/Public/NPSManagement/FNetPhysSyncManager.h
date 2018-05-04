// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"


typedef TScriptInterface<class INetPhysSync> INetPhysSyncPtr;

class FPhysScene;
class FDelegateHandle;
enum EPhysicsSceneType;

/**
 * 
 */
class NETPHYSSYNC_API FNetPhysSyncManager
{
public:
	FNetPhysSyncManager(const AActor* OwningActorParam);
	~FNetPhysSyncManager();

	/**
	 * Lazy initialization to get FPhysScene from UWorld later.
	 */
	void Initialize(FPhysScene* PhysScene);
		
	void RegisterINetPhysSync(INetPhysSyncPtr PtrToUObj);
		
	void UnregisterINetPhysSync(INetPhysSyncPtr PtrToUObj);
	
	/**
	 * Call from actor's PrePhysicTick.
	 * This is call before physic scene determining if there is any sub-step tick.
	 */
	void OnTickPrePhysic();

	/**
	 * Call from actor's PostPhysicTick.
	 */
	void OnTickPostPhysic(float GameFrameDeltaTime);

protected:
	template<typename FunctionArgument>
	void CallINetPhysSyncFunction
	(
		void (INetPhysSync::*Func)(const FunctionArgument&),
		const FunctionArgument& Argument,
		const FIsTickEnableParam& IsTickEnableParam
	)
	{
		for (auto It = INetPhysSyncPtrList.CreateIterator(); It; ++It)
		{
			INetPhysSyncPtr& InterfacePtr = *It;

			if (InterfacePtr != nullptr &&
				InterfacePtr->IsTickEnabled(IsTickEnableParam))
			{
				((*InterfacePtr).*Func)(Argument);
			}
		}
	}

private:
	TArray<INetPhysSyncPtr> INetPhysSyncPtrList;
		
	TArray<INetPhysSyncPtr> DeferedRegister;

	TArray<INetPhysSyncPtr> DeferedUnregister;

	const AActor* OwningActor;

	/**
	 * To check if UWorld owning FPhysScene is not destroyed yet.
	 * Is this necessary?
	 */
	TWeakObjectPtr<UWorld> WorldOwningPhysScene;
		
	FPhysScene* PhysScene;

	uint32 LocalPhysTickIndex;

	bool bStartTickPostPhysicSubstepYet;

	bool bStartPhysicYet;

	float CachStartDeltaTime;

	float CachStepDeltaTime;

	EPhysicsSceneType CachSceneType;

	FDelegateHandle TickStartPhysHandle;

	FDelegateHandle TickStepPhysHandle;

	uint8* PxScratchReplayBuffer;

	int32 PxScratchReplayBufferSize;

	FTickSyncPoint CurrentSyncPoint;

	bool DoWeNeedReplay() const;

	/**
	 * Register to FPhysScene's delegate.
	 */
	void TickStartPhys(FPhysScene* PhysScene, uint32 SceneType, float StartDeltaTime);
		
	/**
	 * Register to FPhysScene's delegate.
	 */
	void TickStepPhys(FPhysScene* PhysScene, uint32 SceneType, float StepDeltaTime);

	void FlushDeferedRegisteeAndCleanNull();

	bool TryGetNewSyncPoint(FTickSyncPoint& OutSyncPoint);

	bool TryGetReplayIndex(uint32& OutReplayIndex);
};
