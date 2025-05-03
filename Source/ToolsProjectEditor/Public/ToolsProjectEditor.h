#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(ToolsProjectEditor, All, All);

class FExtender;

class FToolsProjectEditor : public IModuleInterface
{
	public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

private:
	void MakeCustomMenu(FMenuBarBuilder& MenuBuilder);
	void FillCustomMenu(FMenuBuilder& MenuBuilder);


	/**
	 * A shared pointer to an extensibility manager for the Level Editor
	 * menu.
	 *
	 * This manager allows the modification and extension of the Level
	 * Editor's menu, enabling custom menu entries, actions, and
	 * functionality to be added.
	 */
	TSharedPtr<FExtensibilityManager> LevelEditorMenuExtensibilityManager;
	/**
	 * A shared pointer to a menu extender for the Level Editor menu.
	 *
	 * The menu extender is responsible for extending the existing menu
	 * system by adding custom menu entries or altering the layout of the
	 * menu.
	 */
	TSharedPtr<FExtender> MenuExtender;

};