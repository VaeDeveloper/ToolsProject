// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


struct FStatItem
{
	FText Name;
	FText Num;
	FText Size;
	FText TooltipName;
	FText ToolTipNum;
	FText ToolTipSize;
	FLinearColor TextColor{ FLinearColor::White };
	FMargin NamePadding{ FMargin{0.0f} };
};

struct FAssetTreeFolderNode
{
	FString FolderPath;
	FString FolderName;
	TSharedPtr<FAssetTreeFolderNode> Parent;
	TArray<TSharedPtr<FAssetTreeFolderNode>> SubItems;

	int32 NumAssetsTotal = 0;
	int32 NumAssetsUsed = 0;
	int32 NumAssetsUnused = 0;
	float SizeAssetsUnused = 0.0f;
	float PercentageUnused = 0.0f;
	float PercentageUnusedNormalized = 0.0f;

	bool bIsDev : 1;
	bool bIsRoot : 1;
	bool bIsEmpty : 1;
	bool bIsExcluded : 1;
	bool bIsExpanded : 1;
	bool bIsVisible : 1;

	bool operator==(const FAssetTreeFolderNode& Other) const
	{
		return FolderPath.Equals(Other.FolderPath);
	}

	bool operator!=(const FAssetTreeFolderNode& Other) const
	{
		return !(*this == Other);
	}
};

