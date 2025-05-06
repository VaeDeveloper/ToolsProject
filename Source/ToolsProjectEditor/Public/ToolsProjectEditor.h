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
	/**
	 * Creates and adds a custom menu entry to the main menu bar.
	 *
	 * @param MenuBuilder Reference to the FMenuBarBuilder used to build the menu bar.
	 */
	void MakeCustomMenu(FMenuBarBuilder& MenuBuilder);

	/**
	 * Populates the custom menu with menu items and actions.
	 *
	 * @param MenuBuilder Reference to the FMenuBuilder used to build the menu.
	 */
	void FillManagementMenu(FMenuBuilder& MenuBuilder);
	void FillValidationMenu(FMenuBuilder& MenuBuilder);
	void FillNotepadMenu(FMenuBuilder& MenuBuilder);
	void OnUniversalInputCommitted(const FText& Text, ETextCommit::Type CommitType);
	void SaveTodoNote(const FString& Note);
	void SearchAssets(const FString& Query);
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