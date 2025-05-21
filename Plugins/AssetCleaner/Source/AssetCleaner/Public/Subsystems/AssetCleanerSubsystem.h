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
class ASSETCLEANER_API UAssetCleanerSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	

	/* GetModule */
	static FAssetToolsModule& GetModuleAssetTools();
	static FAssetRegistryModule& GetModuleAssetRegistry();
	static FContentBrowserModule& GetModuleContentBrowser();
	static FPropertyEditorModule& GetModulePropertyEditor();

};
