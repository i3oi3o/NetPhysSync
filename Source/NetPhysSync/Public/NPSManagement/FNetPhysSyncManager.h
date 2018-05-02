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
	FNetPhysSyncManager();
	~FNetPhysSyncManager();

	/**
	 * Lazy initialization to get FPhysScene from UWorld later.
	 */
	void Initialize(FPhysScene* PhysScene);
		
	void RegisterINetPhysSync(INetPhysSyncPtr PtrToUObj);
		
	void UnregisterINetPhysSync(INetPhysSyncPtr PtrToUObj);
		
	/**
	 * Call from actor's PostPhysicTick.
	 */
	void OnTickPostPhysic();

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

	/**
	 * To check if UWorld owning FPhysScene is not destroyed yet.
	 * Is this necessary?
	 */
	TWeakObjectPtr<UWorld> WorldOwningPhysScene;
		
	FPhysScene* PhysScene;

	uint32 LocalPhysTickIndex;

	bool StartTickPostPhysicSubstepYet;

	bool StartPhysicYet;

	float CachStartDeltaTime;

	float CachStepDeltaTime;

	EPhysicsSceneType CachSceneType;

	FDelegateHandle TickStartPhysHandle;

	FDelegateHandle TickStepPhysHandle;

	/**
	 * Register to FPhysScene's delegate.
	 */
	void TickStartPhys(FPhysScene* PhysScene, uint32 SceneType, float StartDeltaTime);
		
	/**
	 * Register to FPhysScene's delegate.
	 */
	void TickStepPhys(FPhysScene* PhysScene, uint32 SceneType, float StepDeltaTime);

	void FlushDeferedRegisteeAndCleanNull();
};
