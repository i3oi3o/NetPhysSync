// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

using UnrealBuildTool;
using Tools.DotNETCommon;
using System;

public class NetPhysSync : ModuleRules
{
	public NetPhysSync(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        int BufferSize = 20;

        Log.TraceInformation("######## Target Directory : "+ Target.ProjectFile.Directory.FullName);

        ConfigHierarchy EngineConfig = ConfigCache.ReadHierarchy
            (
                ConfigHierarchyType.Engine,
                DirectoryReference.FromFile(Target.ProjectFile), 
                Target.Platform 
            );

        ConfigHierarchy GameConfig = ConfigCache.ReadHierarchy
            (
                ConfigHierarchyType.Game,
                DirectoryReference.FromFile(Target.ProjectFile),
                Target.Platform
            );

        if (EngineConfig != null)
        {
            float PhysSubstepDeltaTime;
            float SupportRTTInMS;
            int JitterWaitPhysTick;

            if (EngineConfig.TryGetValue("/Script/Engine.PhysicsSettings", "MaxSubstepDeltaTime", out PhysSubstepDeltaTime) &&
                GameConfig.TryGetValue("/Script/NetPhysSync.NPSNetSetting", "SupportRTTInMS", out SupportRTTInMS) &&
                GameConfig.TryGetValue("/Script/NetPhysSync.NPSNetSetting", "JitterWaitPhysTick", out JitterWaitPhysTick))
            {
                float BufferSizeFloat = (0.001f * SupportRTTInMS) / PhysSubstepDeltaTime;
                BufferSize = (int)Math.Ceiling(BufferSizeFloat);

                if(2*BufferSize > BufferSize + JitterWaitPhysTick)
                {
                    BufferSize = 2 * BufferSize;
                }
                else
                {
                    BufferSize = 2 * BufferSize + JitterWaitPhysTick;
                }

                Log.TraceInformation("######## NPS_BUFFER_SIZE=" + BufferSize);
            }
            else
            {
                Log.TraceInformation("######## Cannot find value from config file. Set NPS_BUFFER_SIZE to default 20");
            }
        }
            
        PublicDefinitions.Add("NPS_BUFFER_SIZE="+BufferSize);
        PublicDefinitions.Add("NPS_LOG_SYNC_AUTO_PROXY=0");

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubSystem", "OnlineSubsystemUtils", "PhysX", "APEX" });


        // Uncomment if you are using Slate UI
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true


	}
}
