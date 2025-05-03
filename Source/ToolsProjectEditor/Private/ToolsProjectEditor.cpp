#include "ToolsProjectEditor.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "DataAssetManager.h"

DEFINE_LOG_CATEGORY(ToolsProjectEditor);

#define LOCTEXT_NAMESPACE "FToolsProjectEditor"

void FToolsProjectEditor::StartupModule()
{
	UE_LOG(ToolsProjectEditor, Warning, TEXT("ToolsProjectEditor module has been loaded"));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorMenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuBarExtension("Help", EExtensionHook::After, nullptr, FMenuBarExtensionDelegate::CreateRaw(this, &FToolsProjectEditor::MakeCustomMenu));
	LevelEditorMenuExtensibilityManager->AddExtender(MenuExtender);
}

void FToolsProjectEditor::ShutdownModule()
{
	UE_LOG(ToolsProjectEditor, Warning, TEXT("ToolsProjectEditor module has been unloaded"));
}

void FToolsProjectEditor::MakeCustomMenu(FMenuBarBuilder& MenuBuilder)
{
	MenuBuilder.AddPullDownMenu(FText::FromString("CustomTools"),
		FText::FromString("Open the Custom menu"),
		FNewMenuDelegate::CreateRaw(this, &FToolsProjectEditor::FillCustomMenu), "CustomTools", FName(TEXT("CustomMenu")));

}

void FToolsProjectEditor::FillCustomMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("Custom Plugin", FText::FromString("Custom Plugin and Tools Section"));
	MenuBuilder.AddMenuEntry(
		FText::FromString("Asset Cleaner Plugin "),
		FText::FromString("Open Asset Cleaner Plugin "),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.ReimportAsset"),
		FUIAction(FExecuteAction::CreateLambda([] {})));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("DataAssetManager", "Data Asset Manager Plugin "),
		LOCTEXT("DataAssetManagerTooltip", "Open Data Asset Manager Plugin "),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.ReimportAsset"),
		FUIAction(FExecuteAction::CreateLambda([] ()
			{
				if(IDataAssetManagerModule* Module = FModuleManager::GetModulePtr<IDataAssetManagerModule>("DataAssetManager"))
				{
					Module->OpenDataAssetManagerTab();
				}
			})));

	MenuBuilder.AddMenuEntry(
		FText::FromString("Blueprint Scanner Plugin"),
		FText::FromString("Open Blueprint Scanner Plugin "),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.ReimportAsset"),
		FUIAction(FExecuteAction::CreateLambda([]{})));

	MenuBuilder.AddMenuEntry(
		FText::FromString("UNotepad Plugin "),
		FText::FromString("Open UNotepad Plugin "),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.ReimportAsset"),
		FUIAction(FExecuteAction::CreateLambda([] {})));

	MenuBuilder.AddMenuEntry(
		FText::FromString("Validation Command Center Plugin "),
		FText::FromString("Open Data Asset Manager Plugin "),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.ReimportAsset"),
		FUIAction(FExecuteAction::CreateLambda([] {})));

	MenuBuilder.EndSection();
}
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToolsProjectEditor, ToolsProjectEditor)