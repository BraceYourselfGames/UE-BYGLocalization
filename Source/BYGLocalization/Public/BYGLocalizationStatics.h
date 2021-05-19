// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BYGLocalizationStatics.generated.h"

UCLASS()
class BYGLOCALIZATION_API UBYGLocalizationGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION( BlueprintCallable )
	static FText GetGameText( const FString& Key );

	UFUNCTION( BlueprintCallable )
	static bool HasTextInTable( const FString& TableName, const FString& Key );
};
