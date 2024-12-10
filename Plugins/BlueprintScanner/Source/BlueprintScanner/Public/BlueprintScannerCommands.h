// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BlueprintScannerStyle.h"

class FBlueprintScannerCommands : public TCommands<FBlueprintScannerCommands>
{
public:

	FBlueprintScannerCommands()
		: TCommands<FBlueprintScannerCommands>(TEXT("BlueprintScanner"), NSLOCTEXT("Contexts", "BlueprintScanner", "BlueprintScanner Plugin"), NAME_None, FBlueprintScannerStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;
	TSharedPtr<FUICommandInfo> ScanAndRefreshButton;
	TSharedPtr<FUICommandInfo> ScanAndRefreshPathButton;
};
