// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FBufferInfo.h"



FBufferInfo::FBufferInfo(uint32 BufferStartTickIndexParam, int32 BufferNumParam)
	: BufferStartTickIndex(BufferStartTickIndexParam)
	, BufferNum(BufferNumParam)
	, BufferLastTickIndex(BufferStartTickIndexParam + BufferNumParam - 1)
{

}

FBufferInfo::~FBufferInfo()
{
}
