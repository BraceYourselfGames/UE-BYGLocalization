// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationSettings.h"
#include "BYGLocalization/Public/BYGLocalization.h"
#include "BYGLocalization/Public/BYGLocalizationModule.h"

UBYGLocalizationSettings::UBYGLocalizationSettings( const FObjectInitializer& ObjectInitializer )
{
	PrimaryLocalizationDirectory.Path = "/Localization/";
}


bool UBYGLocalizationSettings::Validate()
{
	bool bAnyChanges = false;

	if ( PrimaryExtension.IsEmpty() )
	{
		PrimaryExtension = "csv";
		bAnyChanges = true;
	}

	if ( PrimaryLanguageCode.IsEmpty() )
	{
		PrimaryLanguageCode = "en";
		bAnyChanges = true;
	}

	// UE4's localization system expects no / on the end or we get double-slashes
	if ( PrimaryLocalizationDirectory.Path.EndsWith( TEXT( "/" ) ) )
	{
		PrimaryLocalizationDirectory.Path = PrimaryLocalizationDirectory.Path.LeftChop( 1 );
		bAnyChanges = true;
	}

	if ( bUpdateLocsWithCommandLineFlag && CommandLineFlag.IsEmpty() )
	{
		CommandLineFlag = "UpdateLoc";
		bAnyChanges = true;
	}

	return bAnyChanges;
}

#if WITH_EDITOR
void UBYGLocalizationSettings::PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent )
{
	// Rescan for loc files if any path-related properties change
	if ( 
		( PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED( UBYGLocalizationSettings, PrimaryLanguageCode ) )
		|| ( PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED( UBYGLocalizationSettings, PrimaryLocalizationDirectory ) )
		|| ( PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED( UBYGLocalizationSettings, AdditionalLocalizationDirectories ) )
		|| ( PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED( UBYGLocalizationSettings, bIncludeSubdirectories ) )
		|| ( PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED( UBYGLocalizationSettings, FilenamePrefix ) )
		|| ( PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED( UBYGLocalizationSettings, FilenameSuffix ) )
		|| ( PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED( UBYGLocalizationSettings, PrimaryExtension ) )
		|| ( PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED( UBYGLocalizationSettings, AllowedExtensions ) )
		)
	{
		FBYGLocalizationModule::Get().ReloadLocalizations();
	}

	Super::PostEditChangeProperty( PropertyChangedEvent );

}
#endif
