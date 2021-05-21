// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationTest.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "BYGLocalization/Public/BYGLocalizationStatics.h"

#include "Editor/UnrealEd/Public/Tests/AutomationEditorCommon.h"
#include "Developer/FunctionalTesting/Classes/FunctionalTestBase.h"
#include "Core/Public/Misc/FileHelper.h"
#include <Windows/WindowsPlatformProcess.h>
#include <HAL/PlatformFilemanager.h>

static const int TestFlags = (
	EAutomationTestFlags::EditorContext
	| EAutomationTestFlags::CommandletContext
	| EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ProductFilter );


FBYGLocalizationTestBase::FBYGLocalizationTestBase( const FString& InName, const bool bInComplexTask )
	: FFunctionalTestBase( InName, bInComplexTask )
{
	CSVHeader = "Key,SourceString,Comment,English,Status\n";

	// Stuff to test:
	// General CSV stuff
	// - missing table
	// - missing keys
	// - comma run-on
	// - non-ascii
	// - escaping commas

	// UpdateLocalizations
	// - new key detection
	// - updated
	// - deprecated

	TestData = {
		{ "Simple test", {
			"Hello_World,Salut world,",
			{ { "Hello_World", "Salut world" } }
		} },

		// Test loc keys
		{ "Case-sensitivity", {
			"hello_world,Salut world,",
			{ { "Hello_World", "Salut world" } }
		} },
		{ "Spaces in keys", {
			"Hello World,Salut world,",
			{ { "Hello World", "Salut world" } }
		} },
		{ "Quotes around keys", {
			"\"Hello World\",Salut world,",
			{ { "Hello World", "Salut world" } }
		} },
		{ "Quotes around keys with commas", {
			"\"Hello, World\",Salut world,",
			{ { "Hello, World", "Salut world" } }
		} },

		// Loc Values
		{ "Quotes around value with commas", {
			"Hello_World,\"Salut, world\",",
			{ { "Hello_World", "Salut, world" } }
		} },
		{ "No quotes around value with commas", {
			"Hello_World,Salut, world,",
			{ { "Hello_World", "Salut" } }
		} },
		{ "Quotes around value with newline", {
			"Hello_World,\"Salut,\n world\",",
			{ { "Hello_World", "Salut,\n world" } }
		} }
	};
}

void FBYGLocalizationTestBase::GetTests( TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands ) const
{
	for ( const auto& Pair : TestData )
	{
		OutBeautifiedNames.Add( Pair.Key );
		OutTestCommands.Add( Pair.Key );
	}
}

bool FBYGLocalizationTestBase::RunTest( const FString& Parameters )
{
	// Output contents to a temp file
	const auto& Data = TestData[ Parameters ];

	const FString Filename = "test.csv";
	const FString FilenameWithPath = FPaths::Combine( FPlatformProcess::UserTempDir(), Filename );
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if ( PlatformFile.FileExists( *FilenameWithPath ) )
	{
		// delete it
		PlatformFile.DeleteFile( *FilenameWithPath );
	}

	const bool bWriteSucceeded = FFileHelper::SaveStringToFile( CSVHeader + Data.CSVInput, *FilenameWithPath );
	TestTrue( Parameters, bWriteSucceeded );
	const bool bSetFileSucceeded = UBYGLocalizationStatics::SetLocalizationFromFile( FilenameWithPath );
	TestTrue( Parameters, bSetFileSucceeded );

	ensure( Data.ExpectedPairs.Num() > 0 );
	for ( const auto& Pair : Data.ExpectedPairs )
	{
		const FText FoundText = UBYGLocalizationStatics::GetGameText( Pair.Key );
		TestEqual( Parameters, FoundText.ToString(), Pair.Value );
	}

	return true;
}


IMPLEMENT_CUSTOM_COMPLEX_AUTOMATION_TEST( FBYGLocalizationTest, FBYGLocalizationTestBase, "BYG.Localization", TestFlags )
void FBYGLocalizationTest::GetTests( TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands ) const
{
	FBYGLocalizationTestBase::GetTests( OutBeautifiedNames, OutTestCommands );
}

bool FBYGLocalizationTest::RunTest( const FString& Parameters )
{
	return FBYGLocalizationTestBase::RunTest( Parameters );
}

