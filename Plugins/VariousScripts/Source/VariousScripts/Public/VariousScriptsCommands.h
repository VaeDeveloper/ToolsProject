// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "VariousScriptsStyle.h"

class FVariousScriptsCommands : public TCommands<FVariousScriptsCommands>
{
public:

	FVariousScriptsCommands()
		: TCommands<FVariousScriptsCommands>(TEXT("VariousScripts"), NSLOCTEXT("Contexts", "VariousScripts", "VariousScripts Plugin"), NAME_None, FVariousScriptsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
