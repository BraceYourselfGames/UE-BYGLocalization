// Copyright Brace Yourself Games. All Rights Reserved.

using UnrealBuildTool;

public class BYGLocalizationEditor : ModuleRules
{
	public BYGLocalizationEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealEd",
                "UMG",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Slate",
                "SlateCore",
				"BYGLocalization",
				"InputCore",
                "EditorStyle",
				"FunctionalTesting",
            }
        );
	}
}
