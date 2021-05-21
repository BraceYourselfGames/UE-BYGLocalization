// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationEditorModule.h"

#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
#include "Developer/Settings/Public/ISettingsContainer.h"

#include "BYGLocalizationEditor/Private/StatsWindow/BYGLocalizationStatsWindow.h"
#include "Framework/Docking/TabManager.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructure.h"
#include "Widgets/Docking/SDockTab.h"

#include "BYGLocalization/Public/BYGLocalizationSettings.h"

#define LOCTEXT_NAMESPACE "BYGLocalizationEditorModule"

// Not sure why this is necessary, saw it in another module
namespace BYGLocalizationModule
{
	static const FName LocalizationStatsTabName = FName( TEXT( "BYG Localization Stats" ) );
}


TSharedRef<SDockTab> SpawnStatsTab( const FSpawnTabArgs& Args )
{
	return SNew( SDockTab )
		//.Icon( FBYGRichTextUIStyle::GetBrush( "BYGRichText.TabIcon" ) )
		.TabRole( ETabRole::NomadTab )
		.Label( NSLOCTEXT( "BYGLocalization", "StatsTabTitle", "BYG Localization Stats" ) )
		[
			SNew( SBYGLocalizationStatsWindow )
		];
}

void FBYGLocalizationEditorModule::StartupModule()
{
	if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
	{
		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer( "Project" );
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings( "Project", "Plugins", "BYG Localization",
			LOCTEXT( "RuntimeGeneralSettingsName", "BYG Localization" ),
			LOCTEXT( "RuntimeGeneralSettingsDescription", "Simple loc system with support for fan translations." ),
			GetMutableDefault<UBYGLocalizationSettings>()
		);

		if ( SettingsSection.IsValid() )
		{
			SettingsSection->OnModified().BindRaw( this, &FBYGLocalizationEditorModule::HandleSettingsSaved );
		}
	}


	FGlobalTabmanager::Get()->RegisterNomadTabSpawner( BYGLocalizationModule::LocalizationStatsTabName, FOnSpawnTab::CreateStatic( &SpawnStatsTab ) )
		.SetDisplayName( NSLOCTEXT( "BYGLocalization", "TestTab", "BYG Localization Stats" ) )
		.SetTooltipText( NSLOCTEXT( "BYGLocalization", "TestTooltipText", "Open a window with info on localizations." ) )
		.SetGroup( WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory() );
		//.SetIcon( FSlateIcon( FBYGRichTextUIStyle::GetStyleSetName(), "BYGRichText.TabIcon" ) );
}

void FBYGLocalizationEditorModule::ShutdownModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" );

	if ( SettingsModule != nullptr )
	{
		SettingsModule->UnregisterSettings( "Project", "Plugins", "BYG Localizations" );
	}
}

bool FBYGLocalizationEditorModule::HandleSettingsSaved()
{
	UBYGLocalizationSettings* Settings = GetMutableDefault<UBYGLocalizationSettings>();
	const bool ResaveSettings = Settings->Validate();

	if ( ResaveSettings )
	{
		Settings->SaveConfig();
	}

	return true;
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE( FBYGLocalizationEditorModule, BYGLocalizationEditor );


