// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationSettings.h"
#include "BYGLocalization/Public/BYGLocalization.h"

UBYGLocalizationSettings::UBYGLocalizationSettings( const FObjectInitializer& ObjectInitializer )
{
	PrimaryLocalizationDirectory.Path = "/Localization/";

	LocInfo.FoundLocalizations.Empty();

	TArray<FBYGLocalizationLocaleBasic> Locales = UBYGLocalization::GetAvailableLocalizations( false );
	for ( const auto& Loc : Locales )
	{
		LocInfo.FoundLocalizations.Add( Loc.FilePath );
	}
}


FString UBYGLocalizationSettings::RemovePrefixSuffix( const FString& FileWithExtension ) const
{
	FString Filename = FPaths::GetBaseFilename( FileWithExtension );
	if ( !FilenamePrefix.IsEmpty() )
	{
		Filename = Filename.RightChop( FilenamePrefix.Len() );
	}
	if ( !FilenameSuffix.IsEmpty() )
	{
		Filename = Filename.LeftChop( FilenameSuffix.Len() );
	}
	return Filename;
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

	if ( bUpdateLocsWithCommandLineFlag && CommandLineFlag.IsEmpty() )
	{
		CommandLineFlag = "UpdateLocalization";
		bAnyChanges = true;
	}

	return bAnyChanges;
}
