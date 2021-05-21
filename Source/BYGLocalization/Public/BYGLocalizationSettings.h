// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreUObject/Public/UObject/NoExportTypes.h"
#include "BYGLocalizationSettings.generated.h"

UENUM()
enum class EBYGQuotingPolicy : uint8
{
	// Automatically wrap all values with quotation marks
	ForceQuoted,
	// Do not change quotation marks, can result in more runaway strings but fewer diff changes
	OnlyWhenNeeded
};

UENUM()
enum class EBYGPathRoot
{
	// Project root directory. e.g. ProjectName/
	ProjectDir,
	// Project content directory. e.g. ProjectName/Content/
	ContentDir,
	// My Documents. e.g. C:\Users\MyUsername\My Documents
	UserDir,
	// Local app data directory. e.g. "%LocalAppData%\ProjectName"
	UserSettingsDir,
};

USTRUCT()
struct FBYGPath
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere )
	EBYGPathRoot Root = EBYGPathRoot::ContentDir;

	UPROPERTY( EditAnywhere )
	FString RelativePath;

	FDirectoryPath GetDirectoryPath() const
	{
		FDirectoryPath Path;
		switch ( Root )
		{
		case EBYGPathRoot::ContentDir:
			Path.Path = FPaths::ProjectContentDir();
			break;
		case EBYGPathRoot::ProjectDir:
			Path.Path = FPaths::ProjectDir();
			break;
		case EBYGPathRoot::UserDir:
			Path.Path = FPlatformProcess::UserDir(); 
			break;
		case EBYGPathRoot::UserSettingsDir:
			Path.Path = FPlatformProcess::UserSettingsDir(); 
			break;
		}
		Path.Path = FPaths::Combine( Path.Path, RelativePath );
		FPaths::RemoveDuplicateSlashes( Path.Path );
		return Path;
	}
};


UCLASS( config = BYGLocalization, defaultconfig )
class UBYGLocalizationSettings : public UObject
{
	GENERATED_BODY()

public:
	UBYGLocalizationSettings( const FObjectInitializer& ObjectInitializer );

	// Two-character language code used to identify the primary language
	// All other translations will be based on this
	UPROPERTY( config, EditAnywhere, Category = "Language" )
	FString PrimaryLanguageCode = "en";

	// Localization files will be searched for in this directory
	UPROPERTY( config, EditAnywhere, Category = "File Settings", meta = ( ContentDir ) )
	FDirectoryPath PrimaryLocalizationDirectory;

	// Useful for adding any paths to search for fan localization files
	UPROPERTY( config, EditAnywhere, Category = "File Settings", AdvancedDisplay )
	TArray<FBYGPath> AdditionalLocalizationDirectories;

	UPROPERTY( config, EditAnywhere, Category = "File Settings", AdvancedDisplay )
	bool bIncludeSubdirectories = true;

	// Files that start with the prefix, followed by a language code, followed by the suffix, will be matched
	UPROPERTY( config, EditAnywhere, Category = "File Settings" )
	FString FilenamePrefix = "loc_";

	// Files that start with the prefix, followed by a language code, followed by the suffix, will be matched
	UPROPERTY( config, EditAnywhere, Category = "File Settings" )
	FString FilenameSuffix = "";

	bool GetIsValidExtension(const FString& Extension) const
	{
		return Extension == PrimaryExtension
			|| AllowedExtensions.Contains( Extension );
	}

	UPROPERTY( config, EditAnywhere, Category = "File Settings" )
	FString PrimaryExtension = "csv";

	UPROPERTY( config, EditAnywhere, Category = "File Settings" )
	TArray<FString> AllowedExtensions = { "txt" };

	// Creates a backup of the original file when changing any localization files
	UPROPERTY( config, EditAnywhere, Category = "File Settings" )
	bool bCreateBackup = true;

	FString RemovePrefixSuffix( const FString& Filename ) const;

	// This can be used to differentiate between multiple localizations of the same language.
	// e.g. Adding "_meta_author,Fan A,," key to the loc_fr.csv localization file will result in the language showing up as French (Fan A)
	// if there are multiple French localizations
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "CSV Content Settings" )
	FString AuthorMetadataKey = "_meta_author";

	UPROPERTY( config, EditAnywhere, Category = "CSV Content Settings" )
	EBYGQuotingPolicy QuotingPolicy = EBYGQuotingPolicy::ForceQuoted;

	// If a key is longer than this number of characters, show a warning. Useful for finding runaway strings
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "CSV Content Settings" )
	int32 WarnOnLongKey = 100;

	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "CSV Content Settings" )
	bool bWarnOnQuoteFail = true;



	// When true, localization files are updated in non-shipping builds (Devel and Debug)
	UPROPERTY( config, EditAnywhere, Category = "Fan Translation Settings" )
	bool bUpdateLocsInShippingBuilds = false;

	// When true, localization files are updated in non-shipping builds (Devel and Debug)
	UPROPERTY( config, EditAnywhere, Category = "Fan Translation Settings" )
	bool bUpdateLocsInDebugBuilds = true;

	// When true, localization files are only updated when a command-line flag is present. When false, the flag is not required and locs are always updated.
	UPROPERTY( config, EditAnywhere, Category = "Fan Translation Settings" )
	bool bUpdateLocsWithCommandLineFlag = true;

	// Users will have to pass this flag as a command-line argument. e.g. a value of "UpdateLocalization" will mean users will need "-UpdateLocalization" on the command-line
	UPROPERTY( config, EditAnywhere, Category = "Fan Translation Settings", meta = ( EditCondition = "bUpdateLocsWithCommandLineFlag" ) )
	FString CommandLineFlag = "UpdateLocalization";

	// If a key no longer exists in the primary language, any instances of it in other languages are marked "deprecated" in others.
	// If true, all deprecated lines are kept in secondary localizations and marked. If false, they are deleted.
	UPROPERTY( config, EditAnywhere, Category = "CSV Content Settings" )
	bool bPreserveDeprecatedLines = false;





	// WARNING: Changing this string will break any existing FText entries that are saved in Blueprints. Set it once at the start of the project and never change it.
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString StringtableID = "Game";
	// WARNING: Changing this string will break any existing FText entries that are saved in Blueprints. Set it once at the start of the project and never change it.
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString StringtableNamespace = "Namespace";


	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString NewStatus = "New Entry";
	// This is a janky backwards-compatible way so we can snip out the original Original w/o using expensive regexees
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString ModifiedStatusLeft = "Modified Entry: was '";
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString ModifiedStatusRight = "'";
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString DeprecatedStatus = "Deprecated Entry";

	bool Validate();

#if WITH_EDITOR
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override;
#endif
};
