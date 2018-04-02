// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

using UnrealBuildTool;
using System.Collections.Generic;

public class NetPhysSyncTarget : TargetRules
{
	public NetPhysSyncTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "NetPhysSync" } );
	}
}
