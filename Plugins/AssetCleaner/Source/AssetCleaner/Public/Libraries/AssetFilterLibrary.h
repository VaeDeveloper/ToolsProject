// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace AssetCleaner
{
	class ASSETCLEANER_API FAssetFilterLibrary
	{
	public:
		static bool IsAssetUnreferenced(const FAssetData& Asset);
		static bool IsAssetWithMissingReferences(const FAssetData& Asset);
		static void CollectMetadata(const TArray<TSharedPtr<FAssetData>>& InAssetList);
		static void CollectAssetsWithInvalidReferences(const TArray<TSharedPtr<FAssetData>>& InAssetList);
		static void CollectTexturesWithoutCompression(const TArray<TSharedPtr<FAssetData>>& InAssetList);
		static void CollectTexturesWithWrongSize(const TArray<TSharedPtr<FAssetData>>& InAssetList);

		static TSet<FName> AssetsWithMetadata;
		static TSet<FName> TexturesWithoutCompression;
		static TSet<FName> AssetsWithInvalidReferences;
		static TSet<FName> TexturesWithWrongSize;
		static TSet<FName> FilteredMaterials;
	};



}