// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class SDeveloperSettingsWidget;
class SDataAssetManagerWidget;

/* clang-format off */

/**
 * Interface for the Data Asset Manager module.
 * Defines the core API contract for plugin functionality.
 */
class IDataAssetManagerModule : public IModuleInterface 
{
public:
  /**
   * Opens or brings focus to the Data Asset Manager tab
   * @note Pure virtual function must be implemented by derived class
   */
  virtual void OpenDataAssetManagerTab() = 0;
  virtual void RestartWidget() = 0;
};

/**
 * Implementation of the Data Asset Manager module.
 * Handles registration and lifecycle management of editor tab.
 */
class FDataAssetManagerModule : public IDataAssetManagerModule
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Opens or activates the Data Asset Manager tab
	 * @override IDataAssetManagerModule implementation
	 */
	virtual void OpenDataAssetManagerTab() override;

	/**
	 * Restarts/reinitializes the widget component
	 * @note Use when needing to refresh the UI state
	 */
	virtual void RestartWidget() override;

	/**
	 * Static identifier for the Data Asset Manager tab
	 * @return Constant FName identifier used for tab registration
	 */
	static const FName DataAssetManagerTabName;
private:

	ETabSpawnerMenuType::Type GetVisibleModule() const;
	/**
	 * Creates and configures the dockable tab instance
	 * @param Args Spawn arguments provided by tab manager
	 * @return Configured SDockTab instance
	 */
	TSharedRef<SDockTab> CreateDataAssetManagerTab(const FSpawnTabArgs& Args);
	
	/// Widget for developer settings UI (optional component)
	TSharedPtr<SDeveloperSettingsWidget> DeveloperSettingsWidget;
	
	/// Main widget instance for Data Asset management UI
	TSharedPtr<SDataAssetManagerWidget> DataAssetWidget;
};
