using UnrealBuildTool;

public class BYGLocalizationEditor : ModuleRules
{
	public BYGLocalizationEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"UnrealEd",
                "UMG",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
                //"SlateCore",
				"BYGLocalization",
                //"EditorStyle",
				//"SlateCore",
                //"Projects",
				//"PropertyEditor",
				//"EditorSubsystem",
				//"EditorWidgets",
				//"DeveloperSettings",
				//"RenderCore",
				"FunctionalTesting",
				//"DataValidation",
				//"KismetCompiler"
            }
        );

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
			}
		);
	}
}
