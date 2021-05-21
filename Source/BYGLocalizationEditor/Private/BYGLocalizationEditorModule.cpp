// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationEditorModule.h"

#include "PropertyEditorModule.h"
#include "ISettingsModule.h"

//#include "Customization/BYGStyleDisplayTypeCustomization.h"
//#include "Customization/BYGTextJustifyCustomization.h"

#define LOCTEXT_NAMESPACE "BYGLocalizationEditorModule"

void FBYGLocalizationEditor::StartupModule()
{
	FDefaultGameModuleImpl::StartupModule();

	//FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >( "PropertyEditor" );

	//PropertyModule.NotifyCustomizationModuleChanged();

	FCoreDelegates::OnPostEngineInit.AddRaw( this, &FBYGLocalizationEditor::OnPostEngineInit );
}

void FBYGLocalizationEditor::OnPostEngineInit()
{
#if 0
	if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
	{
		SettingsModule->RegisterSettings( "Project", "Plugins", "BYGLocalization",
			LOCTEXT( "RuntimeSettingsName", "BYG Rich Text" ),
			LOCTEXT( "RuntimeSettingsDescription", "Configure the BYG Rich Text plugin" ),
			GetMutableDefault<UBYGLocalizationRuntimeSettings>() );
	}
	#endif
}

void FBYGLocalizationEditor::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();

	//auto& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >( "PropertyEditor" );

	if ( UObjectInitialized() )
	{
		if ( ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>( "Settings" ) )
		{
			SettingsModule->UnregisterSettings( "Project", "Plugins", "BYGLocalization" );
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE( FBYGLocalizationEditor, BYGLocalizationEditor );


