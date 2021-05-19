// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationModule.h"
#include "BYGLocalizationSettings.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Developer/Settings/Public/ISettingsSection.h"
#include "Developer/Settings/Public/ISettingsContainer.h"
#include "Core/Public/Internationalization/StringTableRegistry.h"
#include "BYGLocalization.h"

#define LOCTEXT_NAMESPACE "FBYGLocalizationModule"

void FBYGLocalizationModule::StartupModule()
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
			SettingsSection->OnModified().BindRaw( this, &FBYGLocalizationModule::HandleSettingsSaved );
		}
	}


	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	// GameStrings is the ID we use for our currently-used string table
	// For example it could be French if the player has chosen to use French
	const FString Filename = UBYGLocalization::GetFileWithPathFromLanguageCode( Settings->PrimaryLanguageCode );
	StringTableIDs.Empty();
	StringTableIDs.Add( FName( Settings->StringtableID ) );
	FStringTableRegistry::Get().Internal_LocTableFromFile( StringTableIDs[ 0 ], Settings->StringtableNamespace, Filename, FPaths::ProjectContentDir() );

	// We don't want to register this when we're in editor, because we don't want the 'en' language to be shown when selecting FText in Blueprints
#if !WITH_EDITOR
	// We always keep the localization for the Original language in memory and use it as a fallback in case a string is not found in another language
	{
		StringTableIDs.Add( FName( Settings->PrimaryLanguageCode ) );
		FStringTableRegistry::Get().Internal_LocTableFromFile( StringTableIDs[ 1 ], Settings->StringtableNamespace, Filename, FPaths::ProjectContentDir() );
	}
#endif
}

void FBYGLocalizationModule::ShutdownModule()
{
	if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
	{
		SettingsModule->UnregisterSettings( "Project", "CustomSettings", "General" );
	}

	// Using this because GetDefault<UBYGLocalizationSettings>() is not valid inside ShutdownModule
	for ( const FName& ID : StringTableIDs )
	{
		FStringTableRegistry::Get().UnregisterStringTable( ID );
	}
	StringTableIDs.Empty();
}

bool FBYGLocalizationModule::HandleSettingsSaved()
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

IMPLEMENT_MODULE( FBYGLocalizationModule, BYGLocalization )

