// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

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
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Slate",
                "SlateCore",
				"InputCore",
                "EditorStyle",
				"FunctionalTesting",

				// UIStyle stuff
				"Projects",

				"BYGLocalization",
			}
        );
	}
}
