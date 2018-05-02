// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FStartPhysParam.h"
#include "FPhysStepParam.h"
#include "FPostPhysStepParam.h"
#include "FEndPhysParam.h"
#include "FIsTickEnableParam.h"
#include "INetPhysSync.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNetPhysSync : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NETPHYSSYNC_API INetPhysSync
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
	
};
