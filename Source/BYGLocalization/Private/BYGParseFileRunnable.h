// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "BYGLocalization/Public/BYGLocalization.h"

// Have to use typedef because of macro
DECLARE_DELEGATE_TwoParams( FBYGOnGetStatsCompleteSignature, const FString& /*Path*/, const BYGLocStats& /* Data */);

class BYGLOCALIZATION_API FBYGParseFileRunnable : public FRunnable
{
public:
	FBYGParseFileRunnable( const TArray<FString>& Paths );
	virtual ~FBYGParseFileRunnable();

	// FRunnable functions
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	// FRunnable

	FBYGOnGetStatsCompleteSignature OnGetStatsCompleteSignature;
protected:
	TArray<FString> Paths;

	bool bStopThread = false;
	bool bIsComplete = false;

	class FRunnableThread* Thread;
};
