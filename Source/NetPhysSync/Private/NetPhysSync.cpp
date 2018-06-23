// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

#include "NetPhysSync.h"
#include "Modules/ModuleManager.h"

#if !UE_BUILD_SHIPPING
#include "AutomationTest.h"
#endif

class FNetPhysSyncModuleImpl : public FDefaultGameModuleImpl
{
	virtual void ShutdownModule() override
	{
		FDefaultGameModuleImpl::ShutdownModule();
#if !UE_BUILD_SHIPPING
		// work around hot-reload automation test.
		FAutomationTestFramework& TestFrameWork = FAutomationTestFramework::Get();
		
		// Each name is the class name which we implement in ....Test.cpp
		TestFrameWork.UnregisterAutomationTest(TEXT("FSyncPointTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FShiftReplayPredictionTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FCircularBufferAddingTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FCircularBufferAddingDefaultTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FCircularBufferRemovingTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FCircularBufferInsertTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FUint32OverflowTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FBufferTickOverflowTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FClientActorPredictionHasBufferYetTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FClientActorPredictionSaveAndGetTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FClientActorPredictionShiftBufferTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FClientActorPredictionReplayTickTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FClientPawnPredictionSaveAndGetTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FClientPawnPredictionShiftTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FClientPawnPredictionUnacknowledgedInputTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FClientHandleOverflowCachTick"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FServerHandleOverflowCachTick"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FAutonomousProxyInputConstructorTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FAutoProxySyncCorrectTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FServerPawnUpdateAndCopyTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FServerPawnOutOfOrderPackage"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FServerPawnArriveLatePackage"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FServerPawnObsoletePackage"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FServerPawnSyncClientTickTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FAdaptiveVisualDecayInfoTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FTickIteratorTest"));
		TestFrameWork.UnregisterAutomationTest(TEXT("FReplayStartParamTest"));
#endif
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FNetPhysSyncModuleImpl, NetPhysSync, "NetPhysSync" );

