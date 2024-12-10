// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintScannerCommands.h"

#define LOCTEXT_NAMESPACE "FBlueprintScannerModule"

void FBlueprintScannerCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "BlueprintScanner", "Execute BlueprintScanner action", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ScanAndRefreshButton, "Scanning All Blueprints", "Scanning and Refresh all node in blueprint in project", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ScanAndRefreshPathButton, "Scanning and Refresh all node in blueprints in path folder", "Refresh all nodes in blueprints under this folder", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
