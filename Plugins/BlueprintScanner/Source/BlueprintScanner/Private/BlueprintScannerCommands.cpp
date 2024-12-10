// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintScannerCommands.h"

#define LOCTEXT_NAMESPACE "FBlueprintScannerModule"

void FBlueprintScannerCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "BlueprintScanner", "Execute BlueprintScanner action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
