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
	

	/**
	 * Get a reference to the AssetTools module.
	 * Provides access to asset-related tools and utilities within the editor.
	 *
	 * @return Reference to the FAssetToolsModule instance.
	 */
	static FAssetToolsModule& GetAssetToolsModule();

	/**
	 * Get a reference to the AssetRegistry module.
	 * Used for querying and managing asset metadata and registry information.
	 *
	 * @return Reference to the FAssetRegistryModule instance.
	 */
	static FAssetRegistryModule& GetAssetRegistryModule();

	/**
	 * Get a reference to the ContentBrowser module.
	 * Enables interaction with the Content Browser UI and functionality.
	 *
	 * @return Reference to the FContentBrowserModule instance.
	 */
	static FContentBrowserModule& GetContentBrowserModule();

	/**
	 * Get a reference to the PropertyEditor module.
	 * Provides access to property editing utilities and customization in the editor.
	 *
	 * @return Reference to the FPropertyEditorModule instance.
	 */
	static FPropertyEditorModule& GetPropertyEditorModule();

	/**
	 * Get a reference to the AssetCleaner module.
	 * Custom module for asset cleaning functionalities (specific to this project).
	 *
	 * @return Reference to the FAssetCleanerModule instance.
	 */
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

	/**
	 * Parses a string representing a file size (e.g., "10 MB") and converts it to bytes.
	 *
	 * @param SizeString The size string, including numeric value and unit (KB, MB, GB).
	 * @return The size in bytes as int64.
	 */
	static int64 ParseSizeString(const FString& SizeString);

	/**
	 * Normalizes a file or directory path to an absolute, clean format.
	 *
	 * @param InPath The input path to normalize.
	 * @return The normalized absolute path, or empty string if input is invalid.
	 */
	static FString PathNormalize(const FString& InPath);

	/**
	 * Converts an absolute path to a relative one under the /Game/ virtual root.
	 *
	 * @param InPath The absolute path.
	 * @return A path relative to /Game/, or empty if the path is not under the project.
	 */
	static FString PathConvertToRelative(const FString& InPath);

	/**
	 * Returns the path to the external actors folder under /Game/.
	 *
	 * @return The virtual path to the external actors folder.
	 */
	static FString GetPathExternalActors();

	/**
	 * Returns the path to the external objects folder under /Game/.
	 *
	 * @return The virtual path to the external objects folder.
	 */
	static FString GetPathExternalObjects();

	/**
	 * Determines whether the given path is inside an external actors or objects folder.
	 *
	 * @param InPath The path to check.
	 * @return True if the folder is external; false otherwise.
	 */
	static bool FolderIsExternal(const FString& InPath);

	/**
	 * Converts a relative virtual path under /Game/ to an absolute file system path.
	 *
	 * @param InPath The input path to convert.
	 * @return The absolute path on disk, or empty if conversion fails.
	 */
	static FString PathConvertToAbsolute(const FString& InPath);

	/**
	 * Checks whether the specified folder is completely empty — no assets and no files.
	 *
	 * @param InPath The folder path to check.
	 * @return True if the folder is empty; false if it contains assets or files.
	 */
	static bool FolderIsEmpty(const FString& InPath);

};
