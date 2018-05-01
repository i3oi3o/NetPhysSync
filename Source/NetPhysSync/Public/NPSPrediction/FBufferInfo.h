// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class NETPHYSSYNC_API FBufferInfo
{
public:
	FBufferInfo(uint32 BufferStartTickIndexParam, int32 BufferNumParam);
	~FBufferInfo();

	const uint32 BufferStartTickIndex;
	/**
	 * Or Buffer's Length
	 */
	const int32 BufferNum;

	const uint32 BufferLastTickIndex;
};
