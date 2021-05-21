// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalization.h"
#include "Serialization/Csv/CsvParser.h"
#include "Misc/FileHelper.h"
#include "Internationalization/Regex.h"
#include "BYGLocalizationSettings.h"

DECLARE_LOG_CATEGORY_EXTERN( LogBYGLocalization, Log, All );
DEFINE_LOG_CATEGORY( LogBYGLocalization );

void FBYGLocalizationLocale::SetEntriesInOrder( TArray<FBYGLocalizationEntry>& NewEntries )
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_SetEntriesInOrder );

	EntriesInOrder = NewEntries;
	KeyToIndex.Empty();
	StatusCounts.Empty();

	StatusCounts.Add( EBYGLocalizationStatus::None, 0 );
	StatusCounts.Add( EBYGLocalizationStatus::New, 0 );
	StatusCounts.Add( EBYGLocalizationStatus::Modified, 0 );
	StatusCounts.Add( EBYGLocalizationStatus::Deprecated, 0 );

	// Update key to index stuff
	for ( int32 i = 0; i < EntriesInOrder.Num(); ++i )
	{
		if ( !EntriesInOrder[ i ].bIsEmptyLine )
		{
			// NO DUPLICATE KEYS
			if ( KeyToIndex.Contains( EntriesInOrder[ i ].Key ) )
			{
				UE_LOG( LogBYGLocalization, Warning, TEXT( "Duplicate key found! Line: %d, Key '%s'" ), i, *EntriesInOrder[ i ].Key );
			}
			else
			{
				KeyToIndex.Add( EntriesInOrder[ i ].Key, i );

				StatusCounts[ EntriesInOrder[ i ].Status ] += 1;
			}
		}
	}

}

TArray<FString> UBYGLocalization::GetAllLocalizationFiles()
{
	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

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

bool UBYGLocalization::GetLocalizationExists( const FString& FilePath )
{
	FString FullFilePath = FPaths::Combine( FPaths::ProjectContentDir(), FilePath );

	return FPaths::FileExists( FullFilePath );
}

FString UBYGLocalization::GetFilenameFromLanguageCode( const FString& LanguageCode )
{
	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	return FString::Printf( TEXT( "%s%s%s.%s" ),
		*Settings->FilenamePrefix,
		*LanguageCode,
		*Settings->FilenameSuffix,
		*Settings->PrimaryExtension );
}

FString UBYGLocalization::GetFileWithPathFromLanguageCode( const FString& LanguageCode )
{
	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	return FString::Printf( TEXT( "%s/%s" ),
		*Settings->PrimaryLocalizationDirectory.Path,
		*GetFilenameFromLanguageCode(LanguageCode) );
}

void UBYGLocalization::UpdateTranslations()
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_UpdateTranslations );

	TArray<FString> Files = GetAllLocalizationFiles();

	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	FBYGLocalizationLocale OriginalData;
	for ( const auto& Ext : Settings->AllowedExtensions )
	{
		const FString FullFilename = FString( Settings->PrimaryLanguageCode ).Append( "." ).Append( Ext );
		if ( GetLocalizationExists( FullFilename ) )
		{
			OriginalData = GetLocalizationData( FPaths::Combine( FPaths::ProjectContentDir(), FullFilename ) );

#if !UE_BUILD_SHIPPING
			// Update the Original Language file so that the "Original language" column contains the duplicate Original Language text, for the loc team to be able to copy-paste the
			// loc_en and still have original Original... It doesn't make sense but this is the order from on high.
			auto OriginalDataToWrite = OriginalData.GetEntriesInOrder();
			for ( auto& Entry : OriginalDataToWrite )
			{
				Entry.Original = Entry.Translation;
			}
#endif
			break;
		}
	}
	const auto OriginalEntriesInOrder = OriginalData.GetEntriesInOrder();
	const auto OriginalKeyToIndex = OriginalData.GetKeyToIndex();
	if ( !ensure( OriginalEntriesInOrder.Num() > 0 ) )
		return;

	// Source file is Original
	TArray<FText> LocalizedNames;
	TArray<FString> CultureValues;
	TArray<TArray<FString>> Out;
	for ( const FString& FileWithPath : Files )
	{
		FString CultureName = FPaths::GetBaseFilename( FileWithPath );
		CultureName.ReplaceInline( TEXT( "loc_" ), TEXT( "" ) );

		//if ( CultureName == "en" )
			//continue;
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();


		const FString FullPath = FPaths::Combine( FPaths::ProjectContentDir(), FileWithPath );
		const FFileStatData StatData = PlatformFile.GetStatData( *FullPath );
		if ( StatData.bIsValid && StatData.bIsReadOnly )
		{
			UE_LOG( LogTemp, Warning, TEXT( "Cannot write to read-only file" ) );
			continue;
		}

		FBYGLocalizationLocale LocalData = GetLocalizationData( FullPath );
		const auto LocalEntriesInOrder = LocalData.GetEntriesInOrder();
		const auto LocalKeyToIndex = LocalData.GetKeyToIndex();
		// Find any keys that are missing
		if ( LocalEntriesInOrder.Num() == 0 )
		{
			UE_LOG( LogBYGLocalization, Warning, TEXT( "No Entries found when loading %s" ), *FileWithPath );
		}

		// Will reorder to match
		TArray<FBYGLocalizationEntry> NewEntriesInOrder;

		for ( const FBYGLocalizationEntry& OriginalEntry : OriginalEntriesInOrder )
		{
			if ( OriginalEntry.bIsEmptyLine )
			{
				NewEntriesInOrder.Add( FBYGLocalizationEntry() );
			}
			else
			{
				FBYGLocalizationEntry OldLocalizedEntry;
				if ( LocalKeyToIndex.Contains( OriginalEntry.Key )
					&& LocalKeyToIndex[ OriginalEntry.Key ] >= 0
					&& LocalKeyToIndex[ OriginalEntry.Key ] < LocalEntriesInOrder.Num() )
				{
					OldLocalizedEntry = LocalEntriesInOrder[ LocalKeyToIndex[ OriginalEntry.Key ] ];
				}

				if ( CultureName != "en" )
				{

					FBYGLocalizationEntry NewLocalizedEntry;
					NewLocalizedEntry.bIsEmptyLine = false;
					NewLocalizedEntry.Key = OriginalEntry.Key;
					NewLocalizedEntry.Original = OriginalEntry.Translation;

					if ( OldLocalizedEntry.Translation.IsEmpty() && !OriginalEntry.Translation.IsEmpty() )
					{
						UE_LOG( LogTemp, Warning, TEXT( "%s missing key '%s', adding." ), *CultureName, *OriginalEntry.Key );
						// We want to show Original until they replace the new key with a correct translation, so for now just write in the Original to the translation field
						NewLocalizedEntry.Translation = OriginalEntry.Translation;
						if ( NewLocalizedEntry.Key == "_LocMeta_Author" )
						{
							// Don't copy across author "Brace Yourself Games" for updated translations
							NewLocalizedEntry.Translation = "Unknown";
						}
						NewLocalizedEntry.Status = EBYGLocalizationStatus::New;
					}
					// The display text in the master Original is not the same as the Original in the localization, something was modified
					else if ( OldLocalizedEntry.Original != OriginalEntry.Translation )
					{
						NewLocalizedEntry = OldLocalizedEntry;
						NewLocalizedEntry.Original = OriginalEntry.Translation;
						const FString OldOriginal = OldLocalizedEntry.Original;
						if ( !OldOriginal.IsEmpty() )
						{
							UE_LOG( LogTemp, Warning, TEXT( "Lang %s: Modified key '%s'. Was '%s', now is '%s'" ), *CultureName, *OriginalEntry.Key, *OldOriginal, *OriginalEntry.Translation );
							NewLocalizedEntry.Status = EBYGLocalizationStatus::Modified;
							NewLocalizedEntry.OldOriginal = OldOriginal;
						}
					}
					else
					{
						NewLocalizedEntry = OldLocalizedEntry;
					}

					NewEntriesInOrder.Add( NewLocalizedEntry );
				}
				else
				{
					// Original is output as-is
					OldLocalizedEntry.Original = "";
					NewEntriesInOrder.Add( OldLocalizedEntry );
				}

			}
		}

		for ( const FBYGLocalizationEntry& Entry : LocalEntriesInOrder )
		{
			if ( !OriginalKeyToIndex.Contains( Entry.Key ) )
			{
				// TODO
				UE_LOG( LogTemp, Warning, TEXT( "%s has unused key '%s', marking deprecated." ), *CultureName, *Entry.Key );
				FBYGLocalizationEntry NewEntry = Entry;
				NewEntry.Status = EBYGLocalizationStatus::Deprecated;
				NewEntriesInOrder.Add( NewEntry );
			}
		}

		LocalData.SetEntriesInOrder( NewEntriesInOrder );


		// Output the file
		WriteCSV( LocalData.GetEntriesInOrder(), FPaths::Combine( FPaths::ProjectContentDir(), FileWithPath ) );
	}
}



// Load CSV file into our data structure for ease of use
FBYGLocalizationLocale UBYGLocalization::GetLocalizationData( const FString& Filename )
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_GetLocalizationData );

	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	FBYGLocalizationLocale Locale;
	Locale.FilePath = Filename;

	TArray<FBYGLocalizationEntry> NewEntries;

	FString CSVData;
	if ( FFileHelper::LoadFileToString( CSVData, *Filename ) )
	{
		const FCsvParser Parser( CSVData );
		const FCsvParser::FRows& Rows = Parser.GetRows();

		bool bSkipHeader = true;
		for ( const TArray<const TCHAR*>& Row : Rows )
		{
			if ( bSkipHeader )
			{
				bSkipHeader = false;
				continue;
			}
			if ( Row.Num() >= 2 && FString( Row[ 0 ] ) != "" )
			{
				// Key, Translation, Comment, Original

				// Not everything has a comment
				FString Comment = Row.Num() >= 3 ? FString( Row[ 2 ] ) : "";
				Comment.ReplaceInline( TEXT( "\\" ), TEXT( "" ) );

				FString Key = FString( Row[ 0 ] ).ReplaceEscapedCharWithChar();
				FString Translation = FString( Row[ 1 ] ).ReplaceEscapedCharWithChar();

				// Any remaining backslashes are the bane of my existence
				//Translation.ReplaceInline( TEXT( "\\" ), TEXT( "" ) );

				if ( Key == "_LocMeta_Author" )
				{
					Locale.Author = Translation;
				}

				FRegexPattern RunawayKeyPattern( TEXT( "\n[A-Za-z0-9]+_[A-Za-z0-9_]+," ) );
				FRegexMatcher RunawayMatcher( RunawayKeyPattern, Translation );
				if ( RunawayMatcher.FindNext() )
				{
					UE_LOG( LogBYGLocalization, Warning, TEXT( "Possible runaway quotation mark '%s'" ), *Key );
				}

				FBYGLocalizationEntry Entry( Key, Translation, Comment );
				if ( Row.Num() >= 5 )
				{
					Entry.Original = FString( Row[ 3 ] ).ReplaceEscapedCharWithChar();
					Entry.Original.ReplaceInline( TEXT( "\\" ), TEXT( "" ) );


					FString Status( Row[ 4 ] );

					if ( Status.StartsWith( Settings->DeprecatedStatus ) )
					{
						Entry.Status = EBYGLocalizationStatus::Deprecated;
					}
					else if ( Status.StartsWith( Settings->ModifiedStatusLeft ) )
					{
						Entry.Status = EBYGLocalizationStatus::Modified;
						Entry.OldOriginal = Status.RightChop( Settings->ModifiedStatusLeft.Len() ).LeftChop( Settings->ModifiedStatusRight.Len() );
					}
					else if ( Status.StartsWith( Settings->NewStatus ) )
					{
						Entry.Status = EBYGLocalizationStatus::New;
					}
					else
					{
						Entry.Status = EBYGLocalizationStatus::None;
					}
				}
				NewEntries.Add( Entry );
			}
			else
			{
				// Add dummy/empty
				NewEntries.Add( FBYGLocalizationEntry() );
			}
		}
	}
	else
	{
		UE_LOG( LogBYGLocalization, Warning, TEXT( "Failed to load file '%s'" ), *Filename );
	}

	Locale.SetEntriesInOrder( NewEntries );

	return Locale;
}

FString LazyWrap( const FString& InStr, bool bForceWrap = false )
{
	if ( ( bForceWrap || InStr.Contains( "\"" ) || InStr.Contains( "\r" ) || InStr.Contains( "\n" ) || InStr.Contains(",") )
		&& !InStr.IsEmpty() )
	{
		return "\"" + InStr + "\"";
	}
	else
	{
		return InStr;
	}
}

static const TCHAR* CharToEscapeSeqMap[][2] =
{
	// Always replace \\ first to avoid double-escaping characters
	{ TEXT("\\"), TEXT("\\\\") },
	//{ TEXT("\n"), TEXT("\\n")  },
	//{ TEXT("\r"), TEXT("\\r")  },
	//{ TEXT("\t"), TEXT("\\t")  },
	//{ TEXT("\'"), TEXT("\\'")  },
	{ TEXT("\""), TEXT("\"\"") }
};
static const uint32 MaxSupportedEscapeChars = UE_ARRAY_COUNT( CharToEscapeSeqMap );

/**
 * Replaces certain characters with the "escaped" version of that character (i.e. replaces "\n" with "\\n").
 * The characters supported are: { \n, \r, \t, \', \", \\ }.
 *
 * @param	Chars	by default, replaces all supported characters; this parameter allows you to limit the replacement to a subset.
 *
 * @return	a string with all control characters replaced by the escaped version.
 */
FString UBYGLocalization::ReplaceCharWithEscapedChar( const FString& Str )
{
	if ( Str.Len() > 0 )
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_ReplaceCharWithEscapedChar );

		FString Result(*Str);
		for ( int32 ChIdx = 0; ChIdx < MaxSupportedEscapeChars; ChIdx++ )
		{
			// use ReplaceInline as that won't create a copy of the string if the character isn't found
			Result.ReplaceInline( CharToEscapeSeqMap[ ChIdx ][ 0 ], CharToEscapeSeqMap[ ChIdx ][ 1 ] );
		}
		return Result;
	}

	return *Str;
}

// Write out datastructure to CSV but with our added comment stuff
bool UBYGLocalization::WriteCSV( const TArray<FBYGLocalizationEntry>& Entries, const FString& Filename )
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_BYGLocalization_WriteCSV );

	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	// Ripped from TextLocalizationManager or StringTableCore
	FString ExportedStrings;

	// Write header
	ExportedStrings += TEXT( "Key,SourceString,Comment,Original,Status\n" );

	// Write entries
	for ( const FBYGLocalizationEntry& Entry : Entries )
	{
		if ( Entry.Status == EBYGLocalizationStatus::Deprecated && !Settings->bPreserveDeprecatedLines )
			continue;

		if ( Entry.bIsEmptyLine )
		{
			ExportedStrings += TEXT( "\n" );
		}
		else
		{
			FString ExportedKey = ReplaceCharWithEscapedChar( Entry.Key );
			FString ExportedTranslation = ReplaceCharWithEscapedChar( Entry.Translation );
			FString ExportedComment = ReplaceCharWithEscapedChar( Entry.Comment );
			FString ExportedOriginal = ReplaceCharWithEscapedChar( Entry.Original );

			FString ExportedStatus = "";
			if ( Entry.Status == EBYGLocalizationStatus::Deprecated )
			{
				ExportedStatus = Settings->DeprecatedStatus;
			}
			else if ( Entry.Status == EBYGLocalizationStatus::Modified )
			{
				// Printf is cheaper than concat?
				ExportedStatus = FString::Printf( TEXT( "%s%s%s" ), *Settings->ModifiedStatusLeft, *Entry.OldOriginal, *Settings->ModifiedStatusRight );
			}
			else if ( Entry.Status == EBYGLocalizationStatus::New )
			{
				ExportedStatus = Settings->NewStatus;
			}

			if ( ExportedTranslation != Entry.Translation )
			{
				UE_LOG( LogBYGLocalization, Verbose, TEXT( "Translation wrapped/replaced" ) );
			}

			ExportedStrings += ExportedKey;

			ExportedStrings += TEXT( "," );

			ExportedStrings += LazyWrap( ExportedTranslation, Settings->QuotingPolicy == EBYGQuotingPolicy::ForceQuoted );

			ExportedStrings += TEXT( "," );

			ExportedStrings += LazyWrap( ExportedComment, Settings->QuotingPolicy == EBYGQuotingPolicy::ForceQuoted  );

			ExportedStrings += TEXT( "," );

			ExportedStrings += LazyWrap( ExportedOriginal, Settings->QuotingPolicy == EBYGQuotingPolicy::ForceQuoted  );

			ExportedStrings += TEXT( "," );

			ExportedStrings += LazyWrap( ExportedStatus, Settings->QuotingPolicy == EBYGQuotingPolicy::ForceQuoted  );

			ExportedStrings += TEXT( "\n" );
		}
	}

	return FFileHelper::SaveStringToFile( ExportedStrings, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM );
}


TArray<FBYGLocalizationLocaleBasic> UBYGLocalization::GetAvailableLocalizations( bool bLoadCsvFileContent )
{
	TArray<FBYGLocalizationLocaleBasic> Localizations;

	const TArray<FString> Files = GetAllLocalizationFiles();
	for ( const FString& FileWithPath : Files )
	{
		if ( bLoadCsvFileContent )
		{
			const FString FullPath = FPaths::Combine( FPaths::ProjectContentDir(), FileWithPath );
			// Try loading file
			FString CSVData;
			if ( !FFileHelper::LoadFileToString( CSVData, *FullPath ) )
			{
				UE_LOG( LogBYGLocalization, Error, TEXT( "Failed to load localization file '%s'" ), *FullPath );
				continue;
			}
		}

		FBYGLocalizationLocaleBasic Basic = GetCultureFromFilename( FileWithPath );

		Localizations.Add( Basic );
	}

	return Localizations;
}


bool UBYGLocalization::GetPreferredAvailableCulture( FBYGLocalizationLocaleBasic& FoundLocale )
{
	const TArray<FBYGLocalizationLocaleBasic> AllLocalizations = GetAvailableLocalizations( false );

	// Pretty sure that "language" is what we want, but we will try to fall back to the others..?
	const TArray<FCultureRef> ToTest = {
		FInternationalization::Get().GetDefaultLanguage(),
		FInternationalization::Get().GetDefaultCulture(),
		FInternationalization::Get().GetDefaultLocale()
	};

	for ( const FCultureRef Culture : ToTest )
	{
		UE_LOG( LogBYGLocalization, Warning, TEXT( "%s" ), *Culture.Get().GetName() );
	}

	for ( const FCultureRef Culture : ToTest )
	{
		for ( const FBYGLocalizationLocaleBasic BYGLocal : AllLocalizations )
		{
			UE_LOG( LogBYGLocalization, Warning, TEXT( "%s" ), *Culture.Get().GetName() );
			// Special cases for chinese
			FString LocaleName = Culture.Get().GetTwoLetterISOLanguageName();
			if ( Culture.Get().GetName() == "zh-CN" )
			{
				LocaleName = "cn_HANS";
			}
			else if ( Culture.Get().GetName() == "zh-TW" )
			{
				LocaleName = "cn_HANS";
			}

			if ( LocaleName == BYGLocal.LocaleCode )
			{
				FoundLocale = BYGLocal;
				return true;
			}
		}
	}


	return false;
}

FBYGLocalizationLocaleBasic UBYGLocalization::GetCultureFromFilename( const FString& FileWithPath )
{
	FBYGLocalizationLocaleBasic Basic;
	Basic.FilePath = FileWithPath;

	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();
	const FString CultureName = Settings->RemovePrefixSuffix( FileWithPath );

	Basic.LocaleCode = CultureName;

	FString LocalizedName;

	FCulturePtr FoundCulture = FInternationalization::Get().GetCulture( CultureName );
	if ( FoundCulture.IsValid() )
	{
		Basic.Language = FText::FromString( FoundCulture->GetNativeLanguage() );
	}
	else
	{
		// Couldn't find localized name, just show raw filename
		Basic.Language = FText::FromString( CultureName );
	}

	return Basic;
}

