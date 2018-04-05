
# NetPhysSync
Unreal Project for Network Physic Synchronization Sample.

This project is in progress.

This project requires modified Unreal Engine Source Code with following feature.
 - Physic need to be fixed steps not semi-fixed step.
 - Dispatch physic event, collision, waking etc after each physic sub-step.
 - [FPhysScene::OnPhysScenePreTick](http://api.unrealengine.com/INT/API/Runtime/Engine/FPhysScene/OnPhysScenePreTick/index.html) 
	 - Should only broadcast event if there is physic step in present game frame. 
	 - Only broadcast this event after `FPhysSubstepTask::UpdateTime`. 
	 - The original implementation always broadcast event because it's semi-fixed step.
 - I need API to exposed current sub-step info with following signature. 
	 - `void FPhysScene::GetCurrentSubstepInfo(uint32 SceneType, float& OutSubTime, uint32& OutNumSubsteps, float& OutExcessTimeOutsideSubstep) const`
	 - `void FPhysSubstepTask::GetCurrentSubstepInfo(uint32 SceneType, float& OutSubTime, uint32& OutNumSubsteps, float& OutExcessTimeOutsideSubstep) const`
