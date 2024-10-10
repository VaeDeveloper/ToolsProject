// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "CustomToolsStyle.h"

class FCustomToolsCommands : public TCommands<FCustomToolsCommands>
{
public:

	FCustomToolsCommands()
		: TCommands<FCustomToolsCommands>(TEXT("CustomTools"), NSLOCTEXT("Contexts", "CustomTools", "CustomTools Plugin"), NAME_None, FCustomToolsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
