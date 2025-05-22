// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "AssetCleanerSubsystem.generated.h"


class FAssetToolsModule;
class FAssetRegistryModule;
class FContentBrowserModule;
class FPropertyEditorModule;

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

};
