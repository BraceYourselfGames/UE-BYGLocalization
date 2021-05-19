// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationSettings.h"

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

	return bAnyChanges;
}
