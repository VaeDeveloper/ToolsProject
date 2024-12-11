// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintScannerStyle.h"
#include "BlueprintScanner.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FBlueprintScannerStyle::StyleInstance = nullptr;

void FBlueprintScannerStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FBlueprintScannerStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FBlueprintScannerStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("BlueprintScannerStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FBlueprintScannerStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("BlueprintScannerStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("BlueprintScanner")->GetBaseDir() / TEXT("Resources"));

	Style->Set("BlueprintScanner.PluginAction", new IMAGE_BRUSH(TEXT("Icon"), Icon20x20));
	return Style;
}

void FBlueprintScannerStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FBlueprintScannerStyle::Get()
{
	return *StyleInstance;
}
