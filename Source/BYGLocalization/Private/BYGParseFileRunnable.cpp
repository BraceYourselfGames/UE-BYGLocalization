// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGParseFileRunnable.h"
#include "BYGLocalization/Public/BYGLocalization.h"
#include "HAL/RunnableThread.h"
#include "BYGLocalizationModule.h"


FBYGParseFileRunnable::FBYGParseFileRunnable( const TArray<FString>& InPaths )
{
	Paths = InPaths;
	Thread = FRunnableThread::Create( this, TEXT( "BYGParseFileRunnable" ), 8 * 1024, TPri_Normal );
}

FBYGParseFileRunnable::~FBYGParseFileRunnable()
{
	if ( Thread != nullptr )
	{
		Thread->Kill( true );
		delete Thread;
	}
}

uint32 FBYGParseFileRunnable::Run()
{
	bStopThread = false;

	const UBYGLocalizationSettings* Settings = GetDefault<UBYGLocalizationSettings>();

	// Keep processing until we're cancelled through Stop() or we're done,
	// although this thread will suspended for other stuff to happen at the same time
	while ( !bStopThread && !bIsComplete )
	{
		//TSharedRef<FBYGLocalizationStatEntry> NewItem = FBYGLocalizationStatEntry::Create();
		//NewItem->LocaleCode = Entry.LocaleCode;
		//NewItem->Language = Entry.Language;
		//NewItem->Path = FPaths::Combine( FPaths::ProjectContentDir(), Entry.FilePath );

		// Get stats
		for (int32 i = 0; i < Paths.Num(); ++i )
		{
			BYGLocStats LocStats;
			FBYGLocalizationModule::Get().GetLocalization()->GetLocalizationStats( Paths[ i ], LocStats );
			OnGetStatsCompleteSignature.ExecuteIfBound( Paths[ i ], LocStats );
		}
		bIsComplete = true;
	}

	// Return success
	return 0;
}

void FBYGParseFileRunnable::Exit()
{
	// Here's where we can do any cleanup we want to 
}

void FBYGParseFileRunnable::Stop()
{
	// Force our thread to stop early
	bStopThread = true;
}

