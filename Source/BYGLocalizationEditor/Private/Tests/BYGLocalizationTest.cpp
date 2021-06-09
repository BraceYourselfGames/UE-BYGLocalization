// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "Runtime/Launch/Resources/Version.h"
// Unit testing did not exist before 4.22
#if ENGINE_MINOR_VERSION > 22

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "BYGLocalization/Public/BYGLocalizationStatics.h"
#include "BYGLocalization/Public/BYGLocalization.h"

#include "Editor/UnrealEd/Public/Tests/AutomationEditorCommon.h"
#include "Developer/FunctionalTesting/Classes/FunctionalTestBase.h"
#include "Core/Public/Misc/FileHelper.h"
#include <Windows/WindowsPlatformProcess.h>
#include <HAL/PlatformFilemanager.h>
#include <BYGLocalizationSettings.h>

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

static const int TestFlags = (
	EAutomationTestFlags::EditorContext
	| EAutomationTestFlags::CommandletContext
	| EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ProductFilter );


class UBYGLocalizationSettingsTestProvider : public IBYGLocalizationSettingsProvider
{
public:
	virtual const UBYGLocalizationSettings* GetSettings() const override
	{
		return Blah;
	}
protected:
	UBYGLocalizationSettings* Blah;
};

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST( FBYGLocalizationTest, FFunctionalTestBase, "BYG.Localization.Parse", TestFlags )
bool FBYGLocalizationTest::RunTest( const FString& Parameters )
{
	const FString CSVHeader = "Key,SourceString,Comment,Primary,Status\n";

	struct FBYGTestData
	{
		const FString CSVInput;
		const TMap<FString, FString> ExpectedPairs;
		const bool bExpectFind = true;
	};
	const TMap<FString, FBYGTestData> Data = {
		{ "Simple test", {
			"Hello_World,Salut world,",
			{ { "Hello_World", "Salut world" } }
		} },

		// Test loc keys
		{ "Case-sensitivity", {
			"hello_world,Salut world,",
			{ { "Hello_World", "Salut world" } },
			false
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

	for ( const auto& Pair : Data )
	{
		const FString FilenameWithPath = FPaths::CreateTempFilename( FPlatformProcess::UserTempDir(), TEXT( "BYGLocalizationTest" ), TEXT( ".csv" ) );
		// Output contents to a temp file

		const bool bWriteSucceeded = FFileHelper::SaveStringToFile( CSVHeader + Pair.Value.CSVInput, *FilenameWithPath );
		TestTrue( Pair.Key + " write file", bWriteSucceeded );
		const bool bSetFileSucceeded = UBYGLocalizationStatics::SetLocalizationFromFile( FilenameWithPath );
		TestTrue( Pair.Key + " locfile", bSetFileSucceeded );

		ensure( Pair.Value.ExpectedPairs.Num() > 0 );
		for ( const auto& TextPair : Pair.Value.ExpectedPairs )
		{
			const FText FoundText = UBYGLocalizationStatics::GetGameText( TextPair.Key );
			TestEqual( Pair.Key, FoundText.ToString(), TextPair.Value );
		}
	}

	return true;
}


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST( FBYGEscapeCharacterTest, FFunctionalTestBase, "BYG.Localization.EscapeCharacter", TestFlags )
bool FBYGEscapeCharacterTest::RunTest( const FString& Parameters )
{
	struct FData
	{
		const FString Input;
		const FString ExpectedOutput;
	};
	TMap<FString, FData> Data = {
		{ "Empty string", { "", "" } },
		{ "Single quote", { "\"", "\"\"" } },
		{ "Backslash", { "New\\nLine", "New\\\nLine" } },
		{ "Really long string", { "Then she said \"Hello world,\" and waved.", "Then she said \"\"Hello world,\"\" and waved." } },
	};

	for ( const auto& Pair : Data )
	{
		const FString Output = UBYGLocalization::ReplaceCharWithEscapedChar( Pair.Value.Input );
		TestEqual( Pair.Key, Output, Pair.Value.ExpectedOutput );
	}

	return true;
}


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST( FBYGLazyWrapTest, FFunctionalTestBase, "BYG.Localization.LazyWrap", TestFlags )
bool FBYGLazyWrapTest::RunTest( const FString& Parameters )
{
	struct FData
	{
		const bool bForceWrap;
		const FString Input;
		const FString ExpectedOutput;
	};
	TMap<FString, FData> Data = {
		{ "Basic", { false, "Hello", "Hello" } },
		{ "Lazy space", { false, "Hello World", "Hello World" } },
		{ "Lazy comma", { false, "Hello,World", "\"Hello,World\"" } },
		{ "Lazy newline n", { false, "Hello\nWorld", "\"Hello\nWorld\"" } },
		{ "Lazy newline r", { false, "Hello\rWorld", "\"Hello\rWorld\"" } },
		{ "Lazy double quote", { false, "\"Hello\" World", "\"\"Hello\" World\"" } },
		{ "Lazy single quote", { false, "Hello's World", "Hello's World" } },
		{ "Force wrap", { true, "Hello", "\"Hello\"" } },
		{ "Lazy empty", { false, "", "" } },
		{ "Force wrap empty", { true, "", "" } },
		{ "Force wrap with space", { true, "Hello World", "\"Hello World\"" } },
	};

	for ( const auto& Pair : Data )
	{
		const FString Output = UBYGLocalization::LazyWrap( Pair.Value.Input, Pair.Value.bForceWrap );
		TestEqual( Pair.Key, Output, Pair.Value.ExpectedOutput );
	}

	return true;
}


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST( FBYGWriteCSVTest, FFunctionalTestBase, "BYG.Localization.WriteCSV", TestFlags )
bool FBYGWriteCSVTest::RunTest( const FString& Parameters )
{
	struct FData
	{
		const TArray<FBYGLocalizationEntry> Entries;
		const EBYGQuotingPolicy QuotingPolicy;
		const bool bPreserveDeprecatedLines;
		const FString DeprecatedStatus;
		const FString ModifiedStatusLeft;
		const FString ModifiedStatusRight;
		const FString NewStatus;
		const FString ExpectedOutput;
	};

	// TODO newline \r\n may cause platform issues?
	TMap<FString, FData> Data = {
		{ "Empty", { { }, EBYGQuotingPolicy::OnlyWhenNeeded, false, "Deprecated", "Modified '", "'", "New", "Key,SourceString,Comment,Primary,Status\r\n" } },
		{ "Basic", { { { "Hello", "World", "Nice" } }, EBYGQuotingPolicy::OnlyWhenNeeded, false, "Deprecated", "Modified '", "'", "New", "Key,SourceString,Comment,Primary,Status\r\nHello,World,Nice,,\r\n" } },
		{ "With comma", { { { "Hello", "World, yup", "Nice" } }, EBYGQuotingPolicy::OnlyWhenNeeded, false, "Deprecated", "Modified '", "'", "New", "Key,SourceString,Comment,Primary,Status\r\nHello,\"World, yup\",Nice,,\r\n" } }
	};

	for ( const auto& Pair : Data )
	{
		const FString FilenameWithPath = FPaths::CreateTempFilename( FPlatformProcess::UserTempDir(), TEXT( "BYGLocalizationTest" ), TEXT( ".csv" ) );

		UBYGLocalization* Loc = new UBYGLocalization();
		TSharedRef<IBYGLocalizationSettingsProvider> Provider;
		Loc->Construct( Provider );
		const bool bSuccess = Loc->WriteCSV( Pair.Value.Entries, FilenameWithPath );
		TestTrue( Pair.Key + " file write " + FilenameWithPath, bSuccess );

		FString Output;
		bool bSucceedRead = FFileHelper::LoadFileToString( Output, *FilenameWithPath );
		TestTrue( Pair.Key + " read file " + FilenameWithPath, bSucceedRead );

		TestEqual( Pair.Key + " file contents", Output, Pair.Value.ExpectedOutput );

		delete Loc;
	}

	return true;
}



IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST( FBYGFullLoopTest, FFunctionalTestBase, "BYG.Localization.FullLoop", TestFlags )
bool FBYGFullLoopTest::RunTest( const FString& Parameters )
{
	struct FData
	{
		const FString Input;
	};

	const TArray<FBYGLocalizationEntry> PrimaryEntries = {
		{ "FirstKey", "Hello", "" }
	};
	const TMap<FString, int32> PrimaryKeyToIndex = {
		{ "FirstKey", 0 },
	};

	// TODO test this with the force and lazy quote system
	// TODO newline \r\n may cause platform issues?
	TMap<FString, FData> Data = {
		{ "Single", { "Key,SourceString,Comment,Primary,Status\r\nFirstKey,\"Salut\",\"Just a comment\",\"Hello\",\r\n" } },
		{ "Quotes", { "Key,SourceString,Comment,Primary,Status\r\nFirstKey,\"Hello, world\",\"Just a comment\",\"Hello\",\r\n" } },
		{ "Backslash", { "Key,SourceString,Comment,Primary,Status\r\nFirstKey,\"Hello\\ world\",\"Just a comment\",\"Hello\",\r\n" } },
		// TODO currently unsupported
		// { "Backslashed quote", { "Key,SourceString,Comment,Primary,Status\r\nFirstKey,\"Hello\\\", world\",\"Just a comment\",\"Hello\",\r\n" } },
		{ "Double quote", {
			"Key,SourceString,Comment,Primary,Status\r\nFirstKey,\"She said \"\"Hello\"\", then left.\",\"Just a comment\",\"Hello\",\r\n"
		} },
	};

	// Try processing the files over and over to make sure they don't "drift" and change
	const int32 LoopCount = 3;

	UBYGLocalization* Loc = new UBYGLocalization();
	TSharedRef<IBYGLocalizationSettingsProvider> Provider;
	Loc->Construct( Provider );

	for ( const auto& Pair : Data )
	{
		const FString FilenameWithPath = FPaths::CreateTempFilename( FPlatformProcess::UserTempDir(), TEXT( "BYGLocalizationTest" ), TEXT( ".csv" ) );

		const bool bWriteSucceeded = FFileHelper::SaveStringToFile( Pair.Value.Input, *FilenameWithPath );
		TestTrue( Pair.Key + " write primary file " + FilenameWithPath, bWriteSucceeded );

		for ( int32 i = 0; i < LoopCount; ++i )
		{
			const bool bSuccess = Loc->UpdateTranslationFile( FilenameWithPath, &PrimaryEntries, &PrimaryKeyToIndex );
			TestTrue( Pair.Key + " file write " + FilenameWithPath, bSuccess );

			FString Output;
			const bool bSucceedRead = FFileHelper::LoadFileToString( Output, *FilenameWithPath );
			TestTrue( Pair.Key + " read file " + FilenameWithPath, bSucceedRead );
			TestEqual( Pair.Key + " file contents should be unchanged", Output, Pair.Value.Input );
		}
	}

	delete Loc;

	return true;
}


#endif

