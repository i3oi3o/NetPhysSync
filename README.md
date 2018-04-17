
# NetPhysSync
Unreal Project for Network Physic Synchronization Sample.

This project is in progress. This project use git-lfs to store binary file such .uasset etc.

This project requires modified Unreal Engine Source Code with following feature.
 - Physic need to be fixed steps not semi-fixed step.
 - [FPhysScene::OnPhysScenePreTick](http://api.unrealengine.com/INT/API/Runtime/Engine/FPhysScene/OnPhysScenePreTick/index.html) 
	 - Should only broadcast event if there is physic step in present game frame. 
	 - Only broadcast this event after `FPhysSubstepTask::UpdateTime`. 
	 - The original implementation always broadcast event because it's semi-fixed step.
