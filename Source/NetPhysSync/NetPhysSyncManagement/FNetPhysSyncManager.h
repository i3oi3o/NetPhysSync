// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"


typedef TScriptInterface<class INetPhysSync> INetPhysSyncPtr;

class FPhysScene;
class FDelegateHandle;

/**
 * 
 */
class NETPHYSSYNC_API FNetPhysSyncManager
{
	public:
		FNetPhysSyncManager();
		~FNetPhysSyncManager();

	public:
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

	private:
		TArray<INetPhysSyncPtr> INetPhysSyncPtrList;
		
		/**
		 * To check if UWorld owning FPhysScene is not destroyed yet.
		 * Is this necessary?
		 */
		TWeakObjectPtr<UWorld> WorldOwningPhysScene;
		
		FPhysScene* PhysScene;

		uint32 LocalNetPhysTicks;

		bool StartTickPostPhysicSubstep;

		/**
		 * No Adding or Removing INetPhysSync during PhysicsTick.
		 */
		bool ForAssertDataRace;

		float CachStartDeltaTime;

		float CachStepDeltaTime;

		uint32 CachSceneType;

		FDelegateHandle TickStartPhysHandle;

		FDelegateHandle TickStepPhysHandle;

	private:
		/**
		 * Register to FPhysScene's delegate.
		 */
		void TickStartPhys(FPhysScene* PhysScene, uint32 SceneType, float StartDeltaTime);
		/**
		 * Register to FPhysScene's delegate.
		 */
		void TickStepPhys(FPhysScene* PhysScene, uint32 SceneType, float StepDeltaTime);

		INetPhysSync* TryGetTickableINetPhysSync(const INetPhysSyncPtr& TargetPrt);
};
