// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

template <class InterfaceType> class TScriptInterface;
class INetPhysSync;

/**
 * 
 */
class NETPHYSSYNC_API NPS_StaticHelperFunction
{
public:
	/**
	 * This handles tick overflow.
	 */
	static void CalculateBufferArrayIndex(uint32 BufferStartTickIndex, uint32 BufferTargetIndex, int32& ResultArrayIndex);


	static void UnregisterINetPhySync(TScriptInterface<INetPhysSync> ToUnregister);
};
