// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomToolsCommands.h"

#define LOCTEXT_NAMESPACE "FCustomToolsModule"

void FCustomToolsCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "CustomTools", "Execute CustomTools action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
