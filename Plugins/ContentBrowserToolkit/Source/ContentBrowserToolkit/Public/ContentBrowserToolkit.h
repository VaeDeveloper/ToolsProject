// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"






class FContentBrowserToolkitModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;



private:

	void InitContentBrowserMenuExtension();
	TSharedRef<FExtender> CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths);
	void AddContentBrowserMenuEntry(FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetClicked();
	void FindDuplicateAssets();


	void OnDeleteEmptyFoldersClicked();

	void PopulateAssetActionSubmenu(FMenuBuilder& MenuBuilder);
	void ShowDuplicateAssetsWindow(const TArray<TSharedPtr<struct FDuplicateAssetInfo>>& DuplicateAssets);

	void AddPrefix();

	TArray<FString> FolderPathsSelected;
};
