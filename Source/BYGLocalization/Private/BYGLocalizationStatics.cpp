// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationStatics.h"
#include "BYGLocalizationSettings.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"

bool UBYGLocalizationGameplayStatics::HasTextInTable( const FString& TableName, const FString& Key )
{
	FStringTableConstPtr StringTable = FStringTableRegistry::Get().FindStringTable( *TableName );

	if ( StringTable.IsValid() )
	{
		FStringTableEntryConstPtr Entry = StringTable->FindEntry( *Key );
		return Entry.IsValid();
	}
	return false;
}

FText GetTextFromTable( const FString& TableName, const FString& Key, bool& bIsFound )
{
	bIsFound = false;

	// Not using the default method because on miss it returns something not-useful

	FStringTableConstPtr StringTable = FStringTableRegistry::Get().FindStringTable( *TableName );

	if ( StringTable.IsValid() )
	{

		FStringTableEntryConstPtr pEntry = StringTable->FindEntry( *Key );
		if ( pEntry.IsValid() )
		{
			FTextDisplayStringPtr pStr = pEntry->GetDisplayString();
			bIsFound = true;
			return FText::FromString( *pStr );
		}
		else
		{
			UE_LOG( LogTemp, Warning, TEXT( "Could not find entry %s in string table %s" ), *TableName, *Key );
		}
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "Could not find table %s for key %s" ), *TableName, *Key );
	}
	// Fallback if we didn't find a key
	FString CleanedKey = Key.Replace( TEXT( "_" ), TEXT( "\\_" ) );
	return FText::FromString( FString::Printf( TEXT( "(%s:%s)" ),
		// Trying to use informative compact error message: "key not found" or "table not found"
		StringTable.IsValid() ? *FString( TEXT( "KNF" ) ) : *FString( TEXT( "TNF" ) ),
		*CleanedKey ) );
}

FText UBYGLocalizationGameplayStatics::GetGameText( const FString& Key )
{
	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	bool bFound = false;
	FText Result = GetTextFromTable( Settings->StringtableID, Key, bFound );
	if ( !bFound )
	{
		// Fall back to English if we're not using English and we didn't get the key in the non-English locale  
		Result = GetTextFromTable( Settings->PrimaryLanguageCode, Key, bFound );
	}
	return Result;
}


