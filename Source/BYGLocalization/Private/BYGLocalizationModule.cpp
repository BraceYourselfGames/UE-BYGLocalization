// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationModule.h"
#include "BYGLocalizationSettings.h"
#include "BYGLocalization.h"

#include "Internationalization/StringTableRegistry.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

#define LOCTEXT_NAMESPACE "BYGLocalizationModule"

void FBYGLocalizationModule::StartupModule()
{
	Loc = MakeShareable( new UBYGLocalization() );
	Provider = MakeShareable( new UBYGLocalizationSettingsProvider() );
	Loc->Construct( Provider );

	const UBYGLocalizationSettings* Settings = Provider->GetSettings();

	bool bDoUpdate = false;
	const bool bHasCommandLineFlag = !Settings->bUpdateLocsWithCommandLineFlag || FParse::Param( FCommandLine::Get(), *Settings->CommandLineFlag );
#if UE_BUILD_SHIPPING
	if ( Settings->bUpdateLocsInShippingBuilds )
	{
		bDoUpdate = true && bHasCommandLineFlag;
	}
#else
	if ( Settings->bUpdateLocsInDebugBuilds )
	{
		bDoUpdate = true && bHasCommandLineFlag;
	}
#endif
	if ( bDoUpdate )
	{
		Loc->UpdateTranslations();
	}

	ReloadLocalizations();
}

void FBYGLocalizationModule::ShutdownModule()
{
	// Using this because GetDefault<UBYGLocalizationSettings>() is not valid inside ShutdownModule
	UnloadLocalizations();

	Loc;
}

void FBYGLocalizationModule::ReloadLocalizations()
{
	UnloadLocalizations();

	// TODO provider
	const UBYGLocalizationSettings* Settings = Provider->GetSettings();

	// GameStrings is the ID we use for our currently-used string table
	// For example it could be French if the player has chosen to use French
	const FString Filename = Loc->GetFileWithPathFromLanguageCode( Settings->PrimaryLanguageCode );
	StringTableIDs.Add( FName( *Settings->StringtableID ) );
	FStringTableRegistry::Get().Internal_LocTableFromFile( StringTableIDs[ 0 ], Settings->StringtableNamespace, Filename, FPaths::ProjectContentDir() );

	// We don't want to register this when we're in editor, because we don't want the 'en' language to be shown when selecting FText in Blueprints
#if !WITH_EDITOR
	// We always keep the localization for the Primary language in memory and use it as a fallback in case a string is not found in another language
	{
		StringTableIDs.Add( FName( *Settings->PrimaryLanguageCode ) );
		FStringTableRegistry::Get().Internal_LocTableFromFile( StringTableIDs[ 1 ], Settings->StringtableNamespace, Filename, FPaths::ProjectContentDir() );
	}
#endif
}

void FBYGLocalizationModule::UnloadLocalizations()
{
	// Using this because GetDefault<UBYGLocalizationSettings>() is not valid inside ShutdownModule
	for ( const FName& ID : StringTableIDs )
	{
		FStringTableRegistry::Get().UnregisterStringTable( ID );
	}
	StringTableIDs.Empty();
}

void FBYGLocalizationModule::AddReferencedObjects( FReferenceCollector& Collector )
{
	//Collector.AddReferencedObject( Loc );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FBYGLocalizationModule, BYGLocalization )

