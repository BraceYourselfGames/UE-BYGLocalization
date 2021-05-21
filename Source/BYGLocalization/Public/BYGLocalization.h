// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Internationalization/Culture.h"

enum class EBYGLocalizationStatus : uint8
{
	None,
	New,
	Modified,
	Deprecated
};

struct FBYGLocalizationLocaleBasic
{
	FString LocaleCode;
	FString FilePath;
	FText Language;
};

struct FBYGLocalizationEntry
{
	FBYGLocalizationEntry()
		: bIsEmptyLine( true )
	{
	}
	FBYGLocalizationEntry( FString Key_, FString Translation_, FString Comment_ )
		: Key( Key_ )
		, Translation( Translation_ )
		, Comment( Comment_ )
		, bIsEmptyLine( false )
	{
	}
	FString Key;
	FString Translation;
	FString Comment;
	FString Original;
	FString OldOriginal; // Not in CSV
	EBYGLocalizationStatus Status = EBYGLocalizationStatus::None;
	bool bIsEmptyLine = false;
};

struct FBYGLocalizationLocale
{
public:
	FString FilePath;
	FString Locale;

	FString Author;

	inline TArray<FBYGLocalizationEntry> GetEntriesInOrder() const { return EntriesInOrder; }
	inline TMap<FString, int32> GetKeyToIndex() const { return KeyToIndex; }

	void SetEntriesInOrder( TArray<FBYGLocalizationEntry>& NewEntries );

	TMap<EBYGLocalizationStatus, int32> StatusCounts;

protected:
	TArray<FBYGLocalizationEntry> EntriesInOrder;
	TMap<FString, int32> KeyToIndex;
};

class BYGLOCALIZATION_API UBYGLocalization
{
public:
	// Returns a map from filename to display name
	static TArray<FBYGLocalizationLocaleBasic> GetAvailableLocalizations( bool bLoadCSVFileContent );

	static bool GetLocalizationExists( const FString& FilePath );
	static void UpdateTranslations();

	//static FString GetFilePathForCulture( const FString& Culture, bool bIncludeContentDir );

	static FBYGLocalizationLocale GetLocalizationData( const FString& Filename );

	//static const FString DefaultLocalizationPathWithExtension;

	static bool GetPreferredAvailableCulture( FBYGLocalizationLocaleBasic& FoundLocale );

	static FBYGLocalizationLocaleBasic GetCultureFromFilename( const FString& FileWithPath );

	static FString GetFilenameFromLanguageCode( const FString& LanguageCode );

	static FString GetFileWithPathFromLanguageCode( const FString& LanguageCode );

protected:
	static TArray<FString> GetAllLocalizationFiles();
	static bool WriteCSV( const TArray<FBYGLocalizationEntry>& Entries, const FString& Filename );
	static FString ReplaceCharWithEscapedChar( const FString& Str );

	//static const TArray<FString> LocRootPaths;
	//static const FString NewStatus;
	//static const FString ModifiedStatusLeft;
	//static const FString ModifiedStatusRight;
	//static const FString DeprecatedStatus;
	//static const FString DefaultLocalizationPath;
	//static const TArray<FString> ValidExtensions;

};

