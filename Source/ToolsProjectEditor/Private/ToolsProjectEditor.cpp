#include "ToolsProjectEditor.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "DataAssetManager.h"
#include "ValidatorX.h"
#include "UNotepad.h"
#include "AssetCleaner.h"

DEFINE_LOG_CATEGORY(ToolsProjectEditor);

#define LOCTEXT_NAMESPACE "FToolsProjectEditor"

namespace ToolsProject
{
    // Concept to ensure the module type provides OpenManagerTab().
	template <typename Module>
    // Ensures the module type implements OpenManagerTab().
    concept HasOpenManagerTab = requires(Module module) {
        module.OpenManagerTab();
    };

    /**
     * Utility template function that opens the manager tab of a specified module, if supported.
     *
     * This function checks at compile time if the module type provides an OpenManagerTab() method
     * using the HasOpenManagerTab concept. If the method exists and the module is currently loaded,
     * it invokes the OpenManagerTab() function on the module instance.
     *
     * Example usage:
     * OpenModuleManagerTab<IDataAssetManagerModule>("DataAssetManager");
     *
     * @tparam ModuleType The type of the module interface, typically implementing IModuleInterface.
     * @param ModuleName The registered name of the module, used to retrieve it from FModuleManager.
     */
    template <typename ModuleType>
    void OpenModuleManagerTab(const FName& ModuleName)
    {
        if constexpr(HasOpenManagerTab<ModuleType>)
        {
            if(ModuleType* Module = FModuleManager::GetModulePtr<ModuleType>(ModuleName))
            {
            	Module->OpenManagerTab();
            }
        }
        else
        {
            UE_LOG(ToolsProjectEditor, Warning, TEXT("Module %s does not have OpenManagerTab()"), *ModuleName.ToString());
        }
	}
}


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
//// Adds an invisible menu item to create spacing between other menu entries.
//// This item is non-interactive and cannot be selected. It is used purely for visual separation.
//////////////////////////////////////////////////////////////////////////////////////////////////
//	MenuBuilder.AddMenuEntry(
//		FText::FromString(TEXT("             ")),  
//		FText::FromString(TEXT("Invisible menu item")),
//		FSlateIcon(), 
//		FUIAction(FExecuteAction::CreateLambda([] () {}), FCanExecuteAction::CreateLambda([] () { return false; })));
//////////////////////////////////////////////////////////////////////////////////////////////////

    MenuBuilder.AddPullDownMenu(
        LOCTEXT("ProjectManagementMenu", "Project Management"),
        LOCTEXT("ProjectManagementMenu_Tooltip", "Open the Project Management menu"),
        FNewMenuDelegate::CreateRaw(this, &FToolsProjectEditor::FillManagementMenu),
        "ProjectManagement", FName(TEXT("ProjectManagement")));

    MenuBuilder.AddPullDownMenu(
        LOCTEXT("ValidationMenu", "Validation"),
        LOCTEXT("ValidationMenu_Tooltip", "Open the Validation menu"),
        FNewMenuDelegate::CreateRaw(this, &FToolsProjectEditor::FillValidationMenu),
        "ValidationTools", FName(TEXT("ValidationTools")));

    MenuBuilder.AddPullDownMenu(
        LOCTEXT("NotepadMenu", "Notepad"),
        LOCTEXT("NotepadMenu_Tooltip", "Open the Notepad menu"),
        FNewMenuDelegate::CreateRaw(this, &FToolsProjectEditor::FillNotepadMenu),
        "NotepadTools", FName(TEXT("NotepadTools")));

//// Adds an invisible menu item to create spacing between other menu entries.
//// This item is non-interactive and cannot be selected. It is used purely for visual separation.
//////////////////////////////////////////////////////////////////////////////////////////////////
//    MenuBuilder.AddMenuEntry(
//        FText::FromString(TEXT("                   ")),
//        FText::FromString(TEXT("Invisible menu item")),
//        FSlateIcon(),
//        FUIAction(FExecuteAction::CreateLambda([] () {}), FCanExecuteAction::CreateLambda([] () { return false; })));
//////////////////////////////////////////////////////////////////////////////////////////////////

    //MenuBuilder.AddVerifiedEditableText(
    //    LOCTEXT("UniversalInput", "Universal Input"),
    //    LOCTEXT("UniversalInputTooltip", "Search assets, run editor commands, or save notes.\n"
    //                                  "Examples:\n"
    //                                  "• ChairMesh -> search asset\n"
    //                                  "• cmd:SaveAll -> run editor command\n"
    //                                  "• todo:Fix LODs -> save note"),
    //    FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Search"),
    //    TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([] { return FText::FromString("Initial Text"); })),
    //    FOnVerifyTextChanged::CreateLambda([] (const FText& NewText, FText& OutError) {
    //        if(NewText.ToString().Len() < 3)
    //        {
    //            OutError = FText::FromString("Text too short!");
    //            return false;
    //        }
    //        return true;
    //        }),
    //    FOnTextCommitted::CreateRaw(this, &FToolsProjectEditor::OnUniversalInputCommitted));
}

void FToolsProjectEditor::FillManagementMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.BeginSection("Custom Plugin", LOCTEXT("CustomPluginSection", "Custom Plugin and Tools Section"));

    MenuBuilder.AddMenuEntry(
        LOCTEXT("AssetCleanerPlugin", "Asset Cleaner Plugin"),
        LOCTEXT("AssetCleanerPlugin_Tooltip", "Open Asset Cleaner Plugin"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.ReimportAsset"),
        FUIAction(FExecuteAction::CreateLambda([] { ToolsProject::OpenModuleManagerTab<IAssetCleaner>("AssetCleaner"); })));

    MenuBuilder.AddMenuEntry(
        LOCTEXT("DataAssetManager", "Data Asset Manager Plugin"),
        LOCTEXT("DataAssetManagerTooltip", "Open Data Asset Manager Plugin"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetTreeFolderOpen"),
        FUIAction(FExecuteAction::CreateLambda([] () { ToolsProject::OpenModuleManagerTab<IDataAssetManagerModule>("DataAssetManager"); })));

    MenuBuilder.EndSection();
}

void FToolsProjectEditor::FillValidationMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.BeginSection("ValidationTools", LOCTEXT("ValidationToolsSection", "Tools for data validation"));

    MenuBuilder.AddMenuEntry(
        LOCTEXT("ValidatorXPlugin", "ValidatorX"),
        LOCTEXT("ValidatorXPlugin_Tooltip", "Open ValidatorX Plugin"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"),
        FUIAction(FExecuteAction::CreateLambda([] () { ToolsProject::OpenModuleManagerTab<IValidatorXModule>("ValidatorX"); })));

    MenuBuilder.AddMenuEntry(
        LOCTEXT("BlueprintScannerPlugin", "Blueprint Scanner Plugin"),
        LOCTEXT("BlueprintScannerPlugin_Tooltip", "Open Blueprint Tabs.Recompile"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"),
        FUIAction(FExecuteAction::CreateLambda([] {})));

    MenuBuilder.EndSection();
}

void FToolsProjectEditor::FillNotepadMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.BeginSection("NotepadTools", LOCTEXT("NotepadToolsSection", "Tools for taking notes"));

    MenuBuilder.AddMenuEntry(
        LOCTEXT("UNotepadPlugin", "UNotepad Plugin"),
        LOCTEXT("UNotepadPlugin_Tooltip", "Open UNotepad Plugin"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"),
        FUIAction(FExecuteAction::CreateLambda([] { ToolsProject::OpenModuleManagerTab<IUNotepadModule>("UNotepad"); })));

    MenuBuilder.EndSection();
}

void FToolsProjectEditor::OnUniversalInputCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    FString Input = Text.ToString().TrimStartAndEnd();

    if(Input.StartsWith(TEXT("cmd:")))
    {
        FString Command = Input.Mid(4).TrimStartAndEnd();
        GEditor->Exec(GEditor->GetWorld(), *Command);
        UE_LOG(LogTemp, Log, TEXT("Executed Editor Command: %s"), *Command);
    }
    else if(Input.StartsWith(TEXT("todo:")))
    {
        FString Note = Input.Mid(5).TrimStartAndEnd();
        SaveTodoNote(Note);
        UE_LOG(LogTemp, Log, TEXT("Saved TODO note: %s"), *Note);
    }
    else
    {
        SearchAssets(Input);
    }
}

void FToolsProjectEditor::SaveTodoNote(const FString& Note)
{
    UE_LOG(LogTemp, Log, TEXT("[TODO] %s"), *Note);
}

void FToolsProjectEditor::SearchAssets(const FString& Query)
{
    UE_LOG(LogTemp, Log, TEXT("[Asset Search] Searching for: %s"), *Query);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToolsProjectEditor, ToolsProjectEditor)