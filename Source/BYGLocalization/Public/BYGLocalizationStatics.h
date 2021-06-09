// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BYGLocalizationStatics.generated.h"

UCLASS()
class BYGLOCALIZATION_API UBYGLocalizationStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Primary way for dynamically setting up localized strings. Uses the currently-loaded String Table
	UFUNCTION( BlueprintCallable, Category = "BYG|Localization" )
	static FText GetGameText( const FString& Key );

	// Returns false if either table or text does not exist
	UFUNCTION( BlueprintCallable, Category = "BYG|Localization" )
	static bool HasTextInTable( const FString& TableName, const FString& Key );

	// Use this to change the current localization, for example if the player changes their
	// preferred locale.
	// Need to specify a path and not just a locale code because fan-made localizations
	// can lead to multiple localizations of the same locale.
	UFUNCTION( BlueprintCallable, Category = "BYG|Localization" )
	static bool SetLocalizationFromFile( const FString& Path );
};
