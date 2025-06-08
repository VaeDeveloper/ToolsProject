// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ContentBrowserToolkitSettings.generated.h"


USTRUCT(BlueprintType)
struct FAssetNamingRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, config, Category = "Naming")
	FString Prefix;

	UPROPERTY(EditAnywhere, config, Category = "Naming")
	FString Suffix;
};

/**
 * Asset naming prefixes for different asset types.
 */
UCLASS(config = Editor, defaultconfig, meta = (DisplayName = "Asset Naming Rules"))
class CONTENTBROWSERTOOLKIT_API UContentBrowserToolkitSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Class -> Naming Rule (Prefix & Suffix) */
	UPROPERTY(EditAnywhere, config, Category = "Naming")
	TMap<TSoftClassPtr<UObject>, FAssetNamingRule> ClassNameFormatMap;

	/** Static accessor */
	static const UContentBrowserToolkitSettings* Get()
	{
		return GetDefault<UContentBrowserToolkitSettings>();
	}
};
