// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalization.h"
#include "BYGLocalizationCoreMinimal.h"
#include "BYGLocalizationSettings.h"

#include "Engine/EngineTypes.h"
#include "HAL/PlatformFilemanager.h"
#include "Internationalization/Regex.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/Csv/CsvParser.h"

FBYGLocaleData::FBYGLocaleData( const TArray<FBYGLocalizationEntry>& NewEntries )
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_SetEntriesInOrder );

	EntriesInOrder = NewEntries;
	KeyToIndex.Empty();

	// Update key to index stuff
	for ( int32 i = 0; i < EntriesInOrder.Num(); ++i )
	{
		// NO DUPLICATE KEYS
		if ( KeyToIndex.Contains( EntriesInOrder[ i ].Key ) )
		{
			UE_LOG( LogBYGLocalization, Warning, TEXT( "Duplicate key found! Line: %d, Key '%s'" ), i, *EntriesInOrder[ i ].Key );
		}
		else
		{
			KeyToIndex.Add( EntriesInOrder[ i ].Key, i );
		}
	}
}

void UBYGLocalization::Construct( TSharedPtr<const IBYGLocalizationSettingsProvider> InSettingsProvider )
{
	SettingsProvider = InSettingsProvider;
}

TArray<FString> UBYGLocalization::GetAllLocalizationFiles() const
{
	const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();

	TArray<FString> Files;

	TArray<FDirectoryPath> Paths = { Settings->PrimaryLocalizationDirectory };
	for ( const FBYGPath& BYGPath : Settings->AdditionalLocalizationDirectories )
	{
		Paths.Add( BYGPath.GetDirectoryPath() );
	}
	for ( const FDirectoryPath& Path : Paths )
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString LocalizationDirPath = FPaths::Combine( FPaths::ProjectContentDir(), Path.Path );
		FPaths::RemoveDuplicateSlashes( LocalizationDirPath );
		bool bFound = false;
		TArray<FString> LocalFiles;
		PlatformFile.IterateDirectoryRecursively( *LocalizationDirPath, [&bFound, &PlatformFile, &Files, &Settings]( const TCHAR* InFilenameOrDirectory, const bool bIsDir ) -> bool
		{
			// Find all .txt/.csv files in a dir
			if ( !bIsDir )
			{
				const FString BaseName = FPaths::GetBaseFilename( InFilenameOrDirectory );
				if ( Settings->GetIsValidExtension( FPaths::GetExtension( InFilenameOrDirectory ) )
					&& ( Settings->FilenamePrefix.IsEmpty() || BaseName.StartsWith( Settings->FilenamePrefix ) )
					&& ( Settings->FilenameSuffix.IsEmpty() || BaseName.EndsWith( Settings->FilenameSuffix ) ) )
				{
					FString NewPath = InFilenameOrDirectory;
					FPaths::RemoveDuplicateSlashes( NewPath );
					FPaths::MakePathRelativeTo( NewPath, *FPaths::ProjectContentDir() );
					Files.Add( NewPath );
				}
			}
			// return true to continue searching
			return true;
		} );
		Files.Append( LocalFiles );
	}

	return Files;
}

FString UBYGLocalization::GetFilenameFromLanguageCode( const FString& LanguageCode ) const
{
	const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();

	return FString::Printf( TEXT( "%s%s%s.%s" ),
		*Settings->FilenamePrefix,
		*LanguageCode,
		*Settings->FilenameSuffix,
		*Settings->PrimaryExtension );
}

FString UBYGLocalization::GetFileWithPathFromLanguageCode( const FString& LanguageCode ) const
{
	const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();

	return FString::Printf( TEXT( "%s/%s" ),
		*Settings->PrimaryLocalizationDirectory.Path,
		*GetFilenameFromLanguageCode(LanguageCode) );
}

bool UBYGLocalization::UpdateTranslations()
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_UpdateTranslations );

	const TArray<FString> Files = GetAllLocalizationFiles();

	const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();

	FBYGLocaleData PrimaryData;
	const FString FullFilename = FPaths::Combine( Settings->PrimaryLocalizationDirectory.Path, GetFilenameFromLanguageCode( Settings->PrimaryLanguageCode ) );
	const bool bSucceeded = GetLocalizationDataFromFile( FullFilename, PrimaryData );

	if ( !bSucceeded )
	{
		return false;
	}

	const TArray<FBYGLocalizationEntry>* PrimaryEntriesInOrder = PrimaryData.GetEntriesInOrder();
	const TMap<FString, int32>* PrimaryKeyToIndex = PrimaryData.GetKeyToIndex();
	if ( !ensure( PrimaryEntriesInOrder->Num() > 0 ) )
		return false;

	// Source file is Primary
	for ( const FString& FileWithPath : Files )
	{
		const FString FullPath = FPaths::Combine( FPaths::ProjectContentDir(), FileWithPath );
		UpdateTranslationFile(FileWithPath, PrimaryEntriesInOrder, PrimaryKeyToIndex );
	}

	return true;
}

bool UBYGLocalization::UpdateTranslationFile( const FString& Path,
	const TArray<FBYGLocalizationEntry>* PrimaryEntriesInOrder,
	const TMap<FString, int32>* PrimaryKeyToIndex )
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_UpdateTranslationFile );

	// Source file is Primary
	const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();
	const FString CultureName = RemovePrefixSuffix( Path );

	if ( CultureName == Settings->PrimaryLanguageCode )
		return false;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();


	const FFileStatData StatData = PlatformFile.GetStatData( *Path );
	if ( StatData.bIsValid && StatData.bIsReadOnly )
	{
		UE_LOG( LogBYGLocalization, Warning, TEXT( "Cannot write to read-only file" ) );
		return false;
	}

	FBYGLocaleData LocalData;
	const bool bSucceeded = GetLocalizationDataFromFile( Path, LocalData );
	if ( !bSucceeded )
		return false;
	const TArray<FBYGLocalizationEntry>* LocalEntriesInOrder = LocalData.GetEntriesInOrder();
	const TMap<FString, int32>* LocalKeyToIndex = LocalData.GetKeyToIndex();
	// Find any keys that are missing
	if ( LocalEntriesInOrder->Num() == 0 )
	{
		UE_LOG( LogBYGLocalization, Warning, TEXT( "No Entries found when loading %s" ), *Path );
	}

	// Will reorder to match
	TArray<FBYGLocalizationEntry> NewEntriesInOrder;

	for ( const FBYGLocalizationEntry& PrimaryEntry : *PrimaryEntriesInOrder )
	{
		FBYGLocalizationEntry OldLocalizedEntry;
		if ( LocalKeyToIndex->Contains( PrimaryEntry.Key )
			&& (*LocalKeyToIndex)[ PrimaryEntry.Key ] >= 0
			&& (*LocalKeyToIndex)[ PrimaryEntry.Key ] < LocalEntriesInOrder->Num() )
		{
			OldLocalizedEntry = (*LocalEntriesInOrder)[ (*LocalKeyToIndex)[ PrimaryEntry.Key ] ];
		}

		FBYGLocalizationEntry NewLocalizedEntry;
		NewLocalizedEntry.Key = PrimaryEntry.Key;
		NewLocalizedEntry.Primary = PrimaryEntry.Translation;

		if ( OldLocalizedEntry.Translation.IsEmpty() && !PrimaryEntry.Translation.IsEmpty() )
		{
			UE_LOG( LogBYGLocalization, Warning, TEXT( "%s missing key '%s', adding." ), *CultureName, *PrimaryEntry.Key );
			// We want to show Primary until they replace the new key with a correct translation, so for now just write in the Primary to the translation field
			NewLocalizedEntry.Translation = PrimaryEntry.Translation;
			if ( NewLocalizedEntry.Key == "_LocMeta_Author" )
			{
				// Don't copy across author "Brace Yourself Games" for updated translations
				NewLocalizedEntry.Translation = "Unknown";
			}
			NewLocalizedEntry.Status = EBYGLocEntryStatus::New;
		}
		// The display text in the master Primary is not the same as the Primary in the localization, something was modified
		else if ( OldLocalizedEntry.Primary != PrimaryEntry.Translation )
		{
			NewLocalizedEntry = OldLocalizedEntry;
			NewLocalizedEntry.Primary = PrimaryEntry.Translation;
			const FString OldPrimary = OldLocalizedEntry.Primary;
			if ( !OldPrimary.IsEmpty() )
			{
				UE_LOG( LogBYGLocalization, Warning, TEXT( "Lang %s: Modified key '%s'. Was '%s', now is '%s'" ), *CultureName, *PrimaryEntry.Key, *OldPrimary, *PrimaryEntry.Translation );
				NewLocalizedEntry.Status = EBYGLocEntryStatus::Modified;
				NewLocalizedEntry.OldPrimary = OldPrimary;
			}
		}
		else
		{
			NewLocalizedEntry = OldLocalizedEntry;
		}

		NewEntriesInOrder.Add( NewLocalizedEntry );

	}

	for ( const FBYGLocalizationEntry& Entry : *LocalEntriesInOrder )
	{
		if ( !PrimaryKeyToIndex->Contains( Entry.Key ) )
		{
			// TODO
			UE_LOG( LogBYGLocalization, Warning, TEXT( "%s has unused key '%s', marking deprecated." ), *CultureName, *Entry.Key );
			FBYGLocalizationEntry NewEntry = Entry;
			NewEntry.Status = EBYGLocEntryStatus::Deprecated;
			NewEntriesInOrder.Add( NewEntry );
		}
	}

	// Output the file
	WriteCSV( NewEntriesInOrder, Path );

	return true;
}

// Load CSV file into our data structure for ease of use
bool UBYGLocalization::GetLocalizationDataFromFile( const FString& Filename, FBYGLocaleData& Data ) const
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_GetLocalizationData );

	const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();

	TArray<FBYGLocalizationEntry> NewEntries;

	FString CSVString;
	if ( FFileHelper::LoadFileToString( CSVString, *Filename ) )
	{
		const FCsvParser Parser( CSVString );
		const FCsvParser::FRows& Rows = Parser.GetRows();

		// Validate the header here. Unreal does it for us but we want nicer error-messages
		bool bValidHeader = true;
		if ( Rows.Num() > 0 )
		{
			//Key,SourceString,Comment,Primary,Status
			if ( !Rows[ 0 ].IsValidIndex( 0 ) || FString( Rows[ 0 ][ 0 ] ) != TEXT( "Key" ) )
			{
				UE_LOG( LogBYGLocalization, Error, TEXT( "Column 0 in header must be 'Key'" ) );
				bValidHeader = false;
			}
			if ( !Rows[ 0 ].IsValidIndex( 1 ) || FString( Rows[ 0 ][ 1 ] ) != TEXT( "SourceString" ) )
			{
				UE_LOG( LogBYGLocalization, Error, TEXT( "Column 1 in header must be 'SourceString'" ) );
				bValidHeader = false;
			}
		}
		if ( !bValidHeader )
			return false;

		// Note that we skip the header
		for ( int32 i = 1; i < Rows.Num(); ++i )
		{
			const TArray<const TCHAR*>& Row = Rows[ i ];
			if ( Row.Num() < 2 || FString( Row[ 0 ] ) == "" )
			{
				// Add dummy/empty
			// TODO why?
				NewEntries.Add( FBYGLocalizationEntry() );
				continue;
			}

			// Key,Translation,Comment,Primary,Status

			// Not everything has a comment
			const FString Comment = ( Row.Num() >= 3 ? FString( Row[ 2 ] ) : "" ); //.ReplaceEscapedCharWithChar();

			const FString Key = FString( Row[ 0 ] ); //.ReplaceEscapedCharWithChar();
			const FString Translation = FString( Row[ 1 ] ); //.ReplaceEscapedCharWithChar();

#if 0
			const FRegexPattern RunawayKeyPattern( TEXT( "\n[A-Za-z0-9]+_[A-Za-z0-9_]+," ) );
			FRegexMatcher RunawayMatcher( RunawayKeyPattern, Translation );
			if ( RunawayMatcher.FindNext() )
			{
				UE_LOG( LogBYGLocalization, Warning, TEXT( "Row %d: Possible runaway quotation mark '%s'" ), i, *Key );
			}
#endif

			FBYGLocalizationEntry Entry( Key, Translation, Comment );
			if ( Row.Num() >= 5 )
			{
				Entry.Primary = FString( Row[ 3 ] ); //.ReplaceEscapedCharWithChar();


				const FString Status( Row[ 4 ] );

				if ( Status.StartsWith( Settings->DeprecatedStatus ) )
				{
					Entry.Status = EBYGLocEntryStatus::Deprecated;
				}
				else if ( Status.StartsWith( Settings->ModifiedStatusLeft ) )
				{
					Entry.Status = EBYGLocEntryStatus::Modified;
					Entry.OldPrimary = Status.RightChop( Settings->ModifiedStatusLeft.Len() ).LeftChop( Settings->ModifiedStatusRight.Len() );
				}
				else if ( Status.StartsWith( Settings->NewStatus ) )
				{
					Entry.Status = EBYGLocEntryStatus::New;
				}
				else
				{
					Entry.Status = EBYGLocEntryStatus::None;
				}
			}
			NewEntries.Add( Entry );
		}
	}
	else
	{
		UE_LOG( LogBYGLocalization, Error, TEXT( "Failed to load file '%s'" ), *Filename );
		return false;
	}

	Data = FBYGLocaleData( NewEntries );

	return true;
}

bool UBYGLocalization::GetLocalizationStats( const FString& Filename, BYGLocStats& StatusCounts ) const
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_GetLocalizationStats );

	FString CSVData;
	if ( FFileHelper::LoadFileToString( CSVData, *Filename ) )
	{
		StatusCounts.Empty();

		StatusCounts.Add( EBYGLocEntryStatus::None, 0 );
		StatusCounts.Add( EBYGLocEntryStatus::New, 0 );
		StatusCounts.Add( EBYGLocEntryStatus::Modified, 0 );
		StatusCounts.Add( EBYGLocEntryStatus::Deprecated, 0 );

		const FCsvParser Parser( CSVData );
		const FCsvParser::FRows& Rows = Parser.GetRows();

		const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();

		// Note that we skip the header
		for ( int32 i = 1; i < Rows.Num(); ++i )
		{
			const TArray<const TCHAR*>& Row = Rows[ i ];
			if ( Row.Num() >= 5 )
			{
				const FString Status( Row[ 4 ] );
				if ( Status.StartsWith( Settings->DeprecatedStatus ) )
				{
					StatusCounts[ EBYGLocEntryStatus::Deprecated ] += 1;
				}
				else if ( Status.StartsWith( Settings->ModifiedStatusLeft ) )
				{
					StatusCounts[ EBYGLocEntryStatus::Modified ] += 1;
				}
				else if ( Status.StartsWith( Settings->NewStatus ) )
				{
					StatusCounts[ EBYGLocEntryStatus::New ] += 1;
				}
				else
				{
					StatusCounts[ EBYGLocEntryStatus::None ] += 1;
				}
			}
		}
	}
	else
	{
		UE_LOG( LogBYGLocalization, Error, TEXT( "Failed to load file '%s'" ), *Filename );
		return false;
	}

	return true;
}

FString UBYGLocalization::LazyWrap( const FString& InStr, bool bForceWrap )
{
	if ( ( bForceWrap || InStr.Contains( "\"" ) || InStr.Contains( "\r" ) || InStr.Contains( "\n" ) || InStr.Contains( "," ) )
		&& !InStr.IsEmpty() )
	{
		return "\"" + InStr + "\"";
	}
	else
	{
		return InStr;
	}
}

static const TCHAR* CharToEscapeSeqMap[][ 2 ] =
{
	{ TEXT( "\"" ), TEXT( "\"\"" ) }
};
FString UBYGLocalization::ReplaceCharWithEscapedChar( const FString& Str )
{
	if ( Str.Len() > 0 )
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_ReplaceCharWithEscapedChar );

		FString Result( *Str );
		for ( uint32 ChIdx = 0; ChIdx < 1; ChIdx++ )
		{
			// use ReplaceInline as that won't create a copy of the string if the character isn't found
			Result.ReplaceInline( CharToEscapeSeqMap[ ChIdx ][ 0 ], CharToEscapeSeqMap[ ChIdx ][ 1 ] );
		}
		return Result;
	}

	return *Str;
}


// This is kind of unweidly with all the parameters but it makes testing way easier and it's an internal function
// so what the hell.
bool UBYGLocalization::WriteCSV( const TArray<FBYGLocalizationEntry>& Entries, const FString& Filename )
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_WriteCSV );

	uint32 WriteFlags = 0;
	WriteFlags |= FILEWRITE_EvenIfReadOnly;
	FArchive* CSVFileWriter = IFileManager::Get().CreateFileWriter( *Filename, WriteFlags );
	if ( !CSVFileWriter )
	{
		UE_LOG( LogBYGLocalization, Error, TEXT( "Unable to open csv file \"%s\"." ), *Filename );
		return false;
	}

	const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();

	CSVFileWriter->Logf( TEXT( "Key,SourceString,Comment,Primary,Status" ) );

	const bool bQuote = Settings->QuotingPolicy == EBYGQuotingPolicy::ForceQuoted;

	for ( const FBYGLocalizationEntry& Entry : Entries )
	{
		if ( Entry.Status == EBYGLocEntryStatus::Deprecated && !Settings->bPreserveDeprecatedLines )
			continue;

		// RFC 4180 specifies that double quotes are escaped as ""
		const FString ExportedKey = ReplaceCharWithEscapedChar( Entry.Key );
		const FString ExportedTranslation = ReplaceCharWithEscapedChar( Entry.Translation );
		const FString ExportedComment = ReplaceCharWithEscapedChar( Entry.Comment );
		const FString ExportedPrimary = ReplaceCharWithEscapedChar( Entry.Primary );

		FString ExportedStatus = "";
		if ( Entry.Status == EBYGLocEntryStatus::Deprecated )
		{
			ExportedStatus = Settings->DeprecatedStatus;
		}
		else if ( Entry.Status == EBYGLocEntryStatus::Modified )
		{
			ExportedStatus = FString::Printf( TEXT( "%s%s%s" ), *Settings->ModifiedStatusLeft, *Entry.OldPrimary, *Settings->ModifiedStatusRight );
		}
		else if ( Entry.Status == EBYGLocEntryStatus::New )
		{
			ExportedStatus = Settings->NewStatus;
		}
		ExportedStatus = ReplaceCharWithEscapedChar( ExportedStatus );

		CSVFileWriter->Logf( TEXT( "%s,%s,%s,%s,%s" ),
			*ExportedKey,
			*LazyWrap( ExportedTranslation, bQuote),
			*LazyWrap( ExportedComment, bQuote ),
			*LazyWrap( ExportedPrimary, bQuote ),
			*LazyWrap( ExportedStatus, bQuote ) );
	}

	CSVFileWriter->Close();

	return !CSVFileWriter->IsError();
}


TArray<FBYGLocaleInfo> UBYGLocalization::GetAvailableLocalizations() const
{
	TArray<FBYGLocaleInfo> Localizations;

	const TArray<FString> Files = GetAllLocalizationFiles();
	for ( const FString& FileWithPath : Files )
	{
		FBYGLocaleInfo Basic = GetCultureFromFilename( FileWithPath );
		const FString FullPath = FPaths::Combine( FPaths::ProjectContentDir(), FileWithPath );
		Localizations.Add( Basic );
	}

	return Localizations;
}



bool UBYGLocalization::GetLocaleFromPreferences( FBYGLocaleInfo& FoundLocale ) const
{
	const TArray<FBYGLocaleInfo> AllLocalizations = GetAvailableLocalizations();

	// Pretty sure that "language" is what we want, but we will try to fall back to the others..?
	const TArray<FCultureRef> ToTest = {
		FInternationalization::Get().GetDefaultLanguage(),
		FInternationalization::Get().GetDefaultCulture(),
		FInternationalization::Get().GetDefaultLocale()
	};

	for ( const FCultureRef Culture : ToTest )
	{
		for ( const FBYGLocaleInfo Info : AllLocalizations )
		{
			// Special cases for Chinese
			FString LocaleName = Culture.Get().GetTwoLetterISOLanguageName();
			if ( Culture.Get().GetName() == "zh-CN" )
			{
				LocaleName = "cn_HANS";
			}
			else if ( Culture.Get().GetName() == "zh-TW" )
			{
				LocaleName = "cn_HANS";
			}

			if ( LocaleName == Info.LocaleCode )
			{
				FoundLocale = Info;
				return true;
			}
		}
	}

	return false;
}

FString UBYGLocalization::RemovePrefixSuffix( const FString& FileWithExtension ) const
{
	const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();

	FString Filename = FPaths::GetBaseFilename( FileWithExtension );
	if ( !Settings->FilenamePrefix.IsEmpty() )
	{
		Filename = Filename.RightChop( Settings->FilenamePrefix.Len() );
	}
	if ( !Settings->FilenameSuffix.IsEmpty() )
	{
		Filename = Filename.LeftChop( Settings->FilenameSuffix.Len() );
	}
	return Filename;
}


FBYGLocaleInfo UBYGLocalization::GetCultureFromFilename( const FString& FileWithPath ) const
{
	const UBYGLocalizationSettings* Settings = SettingsProvider->GetSettings();
	const FString LocaleCode = RemovePrefixSuffix( FileWithPath );

	FText LocalizedName;
	const FCulturePtr FoundCulture = FInternationalization::Get().GetCulture( LocaleCode );
	if ( FoundCulture.IsValid() )
	{
		LocalizedName = FText::FromString( FoundCulture->GetNativeLanguage() );
	}
	else
	{
		// Couldn't find localized name, just show raw filename
		LocalizedName = FText::FromString( LocaleCode );
	}
	FBYGLocaleInfo Info {
		LocaleCode,
		LocalizedName,
		FileWithPath
	};

	return Info;
}


bool UBYGLocalization::GetAuthorForLocale( const FString& Filename, FText& Author ) const
{
	//if ( Key == "_LocMeta_Author" )
	//{
		//Data.Author = Translation;
	//}
	return false;
}

