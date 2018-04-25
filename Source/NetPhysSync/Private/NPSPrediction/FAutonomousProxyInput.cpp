// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "FAutonomousProxyInput.h"
#include "FNPS_ClientPawnPrediction.h"

FAutonomousProxyInput::FAutonomousProxyInput
	(
		const FNPS_ClientPawnPrediction& ClientPawnPrediction
	)
{
	ClientPawnPrediction.CopyUnacknowledgedInputToArray
	(
		InputArray, 
		ArrayStartClientTickIndex
	);
}

FAutonomousProxyInput::FAutonomousProxyInput()
	: InputArray()
	, ArrayStartClientTickIndex(0)
{

}

FAutonomousProxyInput::~FAutonomousProxyInput()
{
}
