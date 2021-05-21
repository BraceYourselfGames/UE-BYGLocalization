// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Public/Modules/ModuleManager.h"

class FBYGLocalizationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool SupportsDynamicReloading() override { return true; }

	void ReloadLocalizations();

protected:
	bool HandleSettingsSaved();

	void UnloadLocalizations();

	TArray<FName> StringTableIDs;
};
