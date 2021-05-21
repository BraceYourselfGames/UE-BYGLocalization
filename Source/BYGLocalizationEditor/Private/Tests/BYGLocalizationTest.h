// Copyright Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include <Tests/AutomationEditorCommon.h>
#include <FunctionalTestBase.h>

class FBYGLocalizationTestBase : public FFunctionalTestBase
{

public:
	FBYGLocalizationTestBase( const FString& InName, const bool bInComplexTask );

	virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override;
	virtual bool RunTest( const FString& Parameters ) override;

protected:

	struct FBYGTestData
	{
		const FString CSVInput;
		const TMap<FString, FString> ExpectedPairs;
	};
	TMap<FString, FBYGTestData> TestData;

	FString CSVHeader;
};

