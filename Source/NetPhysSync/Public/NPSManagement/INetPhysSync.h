// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FStartPhysParam.h"
#include "FPhysStepParam.h"
#include "FPostPhysStepParam.h"
#include "FEndPhysParam.h"
#include "FIsTickEnableParam.h"
#include "FReplayStartParam.h"
#include "FReplaySubstepParam.h"
#include "FReplayEndParam.h"
#include "FReplayPostStepParam.h"
#include "FVisualUpdateParam.h"
#include "FOnReadReplicationParam.h"
#include "IQueryReceivedPackage.h"
#include "INetPhysSync.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNetPhysSync : public UQueryReceivedPackage
{
	GENERATED_BODY()
};

/**
 * 
 */
class NETPHYSSYNC_API INetPhysSync : public IQueryReceivedPackage
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual bool IsTickEnabled(const FIsTickEnableParam& param) const PURE_VIRTUAL(INetPhysSync::IsTickEnabled, return false;);

	/**
	 * Call from game thread.
	 */
	virtual void TickStartPhysic(const FStartPhysParam& param) PURE_VIRTUAL(INetPhysSync::TickStartPhysic, );

	/**
	 * Can from game thread.
	 */
	virtual void TickPhysStep(const FPhysStepParam& param) PURE_VIRTUAL(INetPhysSync::TickPhysStep,);

	/**
	 * Can from game thread. 
	 */
	virtual void TickPostPhysStep(const FPostPhysStepParam& param) PURE_VIRTUAL(INetPhysSync::TickPostPhysStep, );

	/**
	 * Call from game thread.
	 */
	virtual void TickEndPhysic(const FEndPhysParam& param) PURE_VIRTUAL(INetPhysSync::TickPhysStep,);
	
	/**
	 * Call from game thread.
	 */
	virtual void TickReplayStart(const FReplayStartParam& param) PURE_VIRTUAL(INetPhysSync::TickReplayStart, );

	/**
	 * Call from game thread.
	 */
	virtual void TickReplaySubstep(const FReplaySubstepParam& param) PURE_VIRTUAL(INetPhysSync::TickReplaySubstep, );

	/**
	 * Call from game thread.
	 */
	virtual void TickReplayPostSubstep(const FReplayPostStepParam& param) PURE_VIRTUAL(INetPhysSync::TickReplayPostSubstep, );

	/**
	 * Call from game thread.
	 */
	virtual void TickReplayEnd(const FReplayEndParam& param) PURE_VIRTUAL(INetPhysSync::TickReplayEnd, );

	/**
	* Call from game thread.
	*/
	virtual void VisualUpdate(const FVisualUpdateParam& param) PURE_VIRTUAL(INetPhysSync::VisualUpdate, );

	/**
	 * Call from game thread.
	 */
	virtual bool TryGetReplayIndex(uint32& OutTickIndex) const PURE_VIRTUAL(INetPhysSync::TryGetReplayIndex, return false; );

	/**
	 * Call from game thread.
	 */
	virtual bool TryGetNewSyncTick(FTickSyncPoint& OutNewSyncPoint) const PURE_VIRTUAL(INetPhysSync::TryGetNewSyncTick, return false; );

	/**
	* Call from game thread.
	*/
	virtual bool IsLocalPlayerControlPawn() const PURE_VIRTUAL(INetPhysSync::IsLocalPlayerControlPawn, return false; );

	/**
	 * Call from game thread.
	 */
	virtual void OnReadReplication(const FOnReadReplicationParam& ReadReplicationParam) PURE_VIRTUAL(INetPhysSync::OnReadReplication, );
};
