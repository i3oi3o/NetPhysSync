// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "TNPSCircularBuffer.h"
#include "FSavedInput.h"
#include "FNPS_ServerPredictionBase.h"

class UNPSNetSetting;

/**
 * 
 */
class NETPHYSSYNC_API FNPS_ServerPawnPrediction : public FNPS_ServerPredictionBase
{
	typedef FNPS_ServerPredictionBase Super;

public:
	FNPS_ServerPawnPrediction(const UNPSNetSetting* const NetSettingParam);
	virtual ~FNPS_ServerPawnPrediction();

	void UpdateInputBuffer
	(
		const struct FAutonomousProxyInput& AutonomousProxyInput,
		uint32 ReceiveServerTickIndex
	);

	const FSavedInput& ProcessServerTick(uint32 ServerTickIndex);



	FORCEINLINE bool HasUnprocessedInput() const
	{
		return InputBuffer.Num() != 0;
	}

	FORCEINLINE bool HasLastProcessedClientTickIndex() const
	{
		return bHasLastProcessedInputClientTickIndex;
	}

	FORCEINLINE bool HasSyncClientTickIndex() const
	{
		return bHasSyncClientTickIndex;
	}

	FORCEINLINE uint32 GetSyncClientTickIndex() const
	{
		return SyncClientTickIndex;
	}

	FORCEINLINE uint32 GetLastProcessedClientTickIndex() const
	{
		return LastProcessedClientTickIndex;
	}

	template<typename AllocatorType>
	void CopyUnprocessedInputToArray
	(
		TArray<FSavedInput, AllocatorType>& DestArray
	)
	{
		DestArray.Empty(DestArray.Max());

		if (HasUnprocessedInput())
		{
			uint32 CurrentUnprocessedServerTick = LastProcessedServerTickIndex + 1;
			int32 UnprocessedArrayIndex = 0;

			FNPS_StaticHelperFunction::CalculateBufferArrayIndex
			(
				CurrentUnprocessedServerTick, 
				InputStartServerTickIndex, 
				UnprocessedArrayIndex
			);

			if (UnprocessedArrayIndex > 0)
			{
				DestArray.AddDefaulted(UnprocessedArrayIndex);
			}
			else if(UnprocessedArrayIndex < 0)
			{
				UnprocessedArrayIndex = -UnprocessedArrayIndex;
			}

			for (int32 i = UnprocessedArrayIndex; i < InputBuffer.Num(); ++i)
			{
				DestArray.Add(InputBuffer[i]);
			}
		}
	}

private :
	TNPSCircularBuffer<FSavedInput, TInlineAllocator<NPS_BUFFER_SIZE>> InputBuffer;
	uint32 InputStartServerTickIndex;
	uint32 InputStartClientTickIndex;
	uint32 LastProcessedClientTickIndex;
	uint32 LastProcessedServerTickIndex;
	uint32 SyncClientTickIndex;
	bool bHasLastProcessedInputClientTickIndex;
	bool bHasLastProcessedServerTickIndex;
	bool bHasSyncClientTickIndex;
	const UNPSNetSetting* NetSetting;
	
};
