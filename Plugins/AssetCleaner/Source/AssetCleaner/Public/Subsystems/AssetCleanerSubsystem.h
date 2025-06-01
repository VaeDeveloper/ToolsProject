// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "AssetCleanerSubsystem.generated.h"


class FAssetToolsModule;
class FAssetRegistryModule;
class FContentBrowserModule;
class FPropertyEditorModule;
class FAssetCleanerModule;
/**
 * 
 */
UCLASS()
class ASSETCLEANER_API UAssetCleanerSubsystem final : public UEditorSubsystem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	

	/* GetModule */
	static FAssetToolsModule& GetAssetToolsModule();
	static FAssetRegistryModule& GetAssetRegistryModule();
	static FContentBrowserModule& GetContentBrowserModule();
	static FPropertyEditorModule& GetPropertyEditorModule();
	static FAssetCleanerModule& GetAssetCleanerModule();
	
	/**
	 * Processes asset data by converting it to asset identifiers and executing a callback.
	 *
	 * @param RefAssetData Array of asset data to process
	 * @param ProcessFunction Callback function that receives processed asset identifiers
	 *
	 * @note Uses IAssetManagerEditorModule to extract identifiers from asset data
	 * @see IAssetManagerEditorModule::ExtractAssetIdentifiersFromAssetDataList()
	 */
	static void ProcessAssetData(const TArray<FAssetData>& RefAssetData, TFunction<void(const TArray<FAssetIdentifier>&)> ProcessFunction);

	/**
	 * Checks if the given folder path should be excluded from asset cleaning.
	 *
	 * This function filters out specific engine/editor-related folders that are not meant to be cleaned,
	 * such as internal or temporary asset management folders.
	 *
	 * @param FolderPath The full path to the folder being checked.
	 * @return true if the folder should be excluded from cleaning; false otherwise.
	 */
	static bool IsExcludedFolder(const FString& FolderPath);

	/**
	 * Deletes multiple assets from the content browser.
	 *
	 * Uses the ObjectTools utility to delete the specified assets and logs the result.
	 * If no assets are provided, logs a warning and returns false.
	 *
	 * @param Assets The array of FAssetData objects representing the assets to delete.
	 * @return true if at least one asset was successfully deleted; false otherwise.
	 */
	static bool DeleteMultiplyAsset(const TArray<FAssetData>& Assets);




};
