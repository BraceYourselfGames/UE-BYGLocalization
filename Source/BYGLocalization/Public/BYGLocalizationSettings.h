// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "BYGLocalizationSettings.generated.h"

USTRUCT()
struct FBYGLocalizationInfo
{
	GENERATED_BODY()
public:
	UPROPERTY( VisibleAnywhere )
	FString Test;

};

UENUM()
enum class EBYGQuotingPolicy : uint8
{
	ForceQuoted,
	OnlyWhenNeeded
};

UCLASS( config = BYGLocalization, defaultconfig )
class UBYGLocalizationSettings : public UObject
{
	GENERATED_BODY()

public:
	// Two-character language code used to identify the primary language
	// All other translations will be based on this
	UPROPERTY( config, EditAnywhere, Category = "Language" )
	FString PrimaryLanguageCode = "en";

	UPROPERTY( config, EditAnywhere, Category = "File Settings" )
	FDirectoryPath PrimaryLocalizationDirectory;

	UPROPERTY( config, EditAnywhere, Category = "File Settings" )
	TArray<FDirectoryPath> AdditionalLocalizationDirectories;

	UPROPERTY( config, EditAnywhere, Category = "File Settings" )
	FString FilenamePrefix = "loc_";

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

	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "CSV Content Settings" )
	FString AuthorMetadataKey = "_meta_author";

	UPROPERTY( config, EditAnywhere, Category = "CSV Content Settings" )
	EBYGQuotingPolicy QuotingPolicy = EBYGQuotingPolicy::ForceQuoted;

	UPROPERTY( config, EditAnywhere, Category = "CSV Content Settings" )
	bool bPreserveDeprecatedLines = false;

	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "CSV Content Settings" )
	int32 WarnOnLongKey = 100;

	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "CSV Content Settings" )
	bool bWarnOnQuoteFail = true;


	// WARNING: Changing this string will break any existing FText entries that are saved in Blueprints. Set it once at the start of the project and never change it.
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString StringtableID = "Game";
	// WARNING: Changing this string will break any existing FText entries that are saved in Blueprints. Set it once at the start of the project and never change it.
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString StringtableNamespace = "Namespace";


	// 
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString NewStatus = "New Entry";

	// This is a janky backwards-compatible way so we can snip out the original Original w/o using expensive regexees
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString ModifiedStatusLeft = "Modified Entry: was '";
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString ModifiedStatusRight = "'";
	UPROPERTY( config, EditAnywhere, AdvancedDisplay, Category = "Internal Settings" )
	FString DeprecatedStatus = "Deprecated Entry";


	UPROPERTY( VisibleAnywhere, Category = "Statistics" )
	FBYGLocalizationInfo LocInfo;

	bool Validate();
};
