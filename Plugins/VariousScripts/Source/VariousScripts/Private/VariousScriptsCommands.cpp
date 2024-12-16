// Copyright Epic Games, Inc. All Rights Reserved.

#include "VariousScriptsCommands.h"

#define LOCTEXT_NAMESPACE "FVariousScriptsModule"

void FVariousScriptsCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "VariousScripts", "Execute VariousScripts action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
