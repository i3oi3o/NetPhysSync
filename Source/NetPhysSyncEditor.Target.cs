// This is licensed under the BSD License 2.0 found in the LICENSE file in project's root directory.

using UnrealBuildTool;
using System.Collections.Generic;

public class NetPhysSyncEditorTarget : TargetRules
{
	public NetPhysSyncEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "NetPhysSync" } );
	}
}
