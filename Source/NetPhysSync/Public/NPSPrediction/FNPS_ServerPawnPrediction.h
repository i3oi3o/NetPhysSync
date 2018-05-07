// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#pragma once

#include "CoreMinimal.h"
#include "TNPSCircularBuffer.h"
#include "FSavedInput.h"
#include "FBufferInfo.h"
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

	/**
	 * Should be called every tick.
	 */
	const FSavedInput& ProcessServerTick(uint32 ServerTickIndex);

	/**
	 * If we process all input but doesn't received input that marking the end of input,
	 * This return true.
	 */
	FORCEINLINE bool IsProcessingClientInput() const
	{
		return InputBuffer.Num() > 0;
	}

	bool HasUnprocessedInputForSimulatedProxy() const;

	FORCEINLINE FBufferInfo GetInputServerTickBufferInfo() const
	{
		return FBufferInfo(InputStartServerTickIndex, InputBuffer.Num());
	}

	FORCEINLINE bool HasLastProcessedClientTickIndex() const
	{
		return bHasLastProcessedInputClientTickIndex;
	}

	FORCEINLINE bool HasSyncClientTickIndex() const
	{
		return bHasSyncClientTickIndex;
	}

	/**
	 * This is sync with FNetPhysSyncManager::LocalPhysTickIndex as long as you call
	 * FNPS_ServerPawnPrediction::ProcessServerTick(uint32) every tick.
	 */
	FORCEINLINE uint32 GetSyncClientTickIndexForStampRigidBody() const
	{
		return SyncClientTickIndexForStampRigidBody;
	}

	FORCEINLINE uint32 GetLastProcessedClientInputTickIndex() const
	{
		return LastProcessedClientInputTickIndex;
	}

	template<typename AllocatorType>
	void CopyUnprocessedInputForSimulatedProxyToArray
	(
		TArray<FSavedInput, AllocatorType>& DestArray
	)
	{
		DestArray.Empty(DestArray.Max());

		if (IsProcessingClientInput())
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
				UnprocessedArrayIndex = 0;
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
	uint32 LastProcessedClientInputTickIndex;
	uint32 LastProcessedServerTickIndex;
	uint32 SyncClientTickIndexForStampRigidBody;
	bool bHasLastProcessedInputClientTickIndex;
	bool bHasLastProcessedServerTickIndex;
	bool bHasSyncClientTickIndex;
	const UNPSNetSetting* NetSetting;
	
};
