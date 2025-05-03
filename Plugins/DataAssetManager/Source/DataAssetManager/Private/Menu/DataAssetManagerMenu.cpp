// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/DataAssetManagerMenu.h"
#include "RevisionControlStyle/RevisionControlStyle.h"

#define LOCTEXT_NAMESPACE "SDataAssetManagerWidget"

#define CREATE_ACTION(Manager, Method) \
	FUIAction(FExecuteAction::CreateSP(Manager, Method))

#define CREATE_ACTION_WITH_PARAM(Manager, Method, Param) \
	FUIAction(FExecuteAction::CreateSP(Manager, Method, Param))

#define CREATE_DELEGATE_ACTION(Manager, Method) \
	FNewMenuDelegate::CreateStatic(Method, Manager)

#define CREATE_ACTION_WITH_CAN_EXECUTE(Manager, ExecuteMethod, CanExecuteMethod) \
	FUIAction( \
		FExecuteAction::CreateSP(Manager, ExecuteMethod), \
		FCanExecuteAction::CreateSP(Manager, CanExecuteMethod) \
	)


namespace DataAssetManagerMenu
{
	namespace IconStyle
	{
		static const FName AppStyle = FAppStyle::GetAppStyleSetName();
		static const FName RevisionControlStyle = FRevisionControlStyleManager::GetStyleSetName();
	}

	namespace Icons
	{
		// File Menu
		const FSlateIcon AddNewAsset = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.AssetActions.ReimportAsset");
		const FSlateIcon SaveAsset = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.SaveAllCurrentFolder");
		const FSlateIcon SaveAll = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.SaveAllCurrentFolder");
		const FSlateIcon Validate = FSlateIcon(IconStyle::AppStyle, "Icons.Adjust");
		const FSlateIcon Rename = FSlateIcon(IconStyle::AppStyle, "GenericCommands.Rename");
		const FSlateIcon Delete = FSlateIcon(IconStyle::AppStyle, "GenericCommands.Delete");
		// Assets Menu
		const FSlateIcon OpenAsset = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.ShowInExplorer");
		const FSlateIcon FindInCB = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.ShowInExplorer");
		const FSlateIcon Copy = FSlateIcon(IconStyle::AppStyle, "GenericCommands.Copy");
		const FSlateIcon ReferenceViewer = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.ReferenceViewer");
		const FSlateIcon SizeMap = FSlateIcon(IconStyle::AppStyle, "ContentBrowser.SizeMap");
		const FSlateIcon Audit = FSlateIcon(IconStyle::AppStyle, "Icons.Audit");
		const FSlateIcon RevisionControl = FSlateIcon(IconStyle::RevisionControlStyle, "RevisionControl.Actions.Diff");

		// Settings Menu
		const FSlateIcon MessageLog = FSlateIcon(IconStyle::AppStyle, "MessageLog.TabIcon");
		const FSlateIcon Visibility = FSlateIcon(IconStyle::AppStyle, "Icons.Visibility");
		const FSlateIcon Settings = FSlateIcon(IconStyle::AppStyle, "Icons.Settings");
		const FSlateIcon Refresh = FSlateIcon(IconStyle::AppStyle, "Icons.Refresh");
		const FSlateIcon OutputLog = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Log.TabIcon");

		// Help Menu
		const FSlateIcon Documentation = FSlateIcon(IconStyle::AppStyle, "GraphEditor.GoToDocumentation");
	}

	namespace Names
	{
		const FName ExtensionHookCreateName = TEXT("Created");
		const FName ExtensionHookEditName = TEXT("Edit");
		const FName ExtensionHookValidateName = TEXT("Validate");
		const FName ExtensionHookDebugName = TEXT("Debug");
		const FName ExtensionHookSettingsName = TEXT("Settings");
		const FName ExtensionHookPluginSettingsName = TEXT("PluginSettings");
		const FName ExtensionHookRestartName = TEXT("Restart");
	}

	namespace Texts
	{
		// File Menu
		const FText CreateSectionText = LOCTEXT("CreateSection", "Create");
		const FText AddNewAssetText = LOCTEXT("AddNewAsset", "Add New Data Asset");
		const FText AddNewAssetTooltip = LOCTEXT("AddNewAssetTooltip", "Create new Data Asset in Content Browser");
		const FText EditSectionText = LOCTEXT("EditSection", "Edit");
		const FText RenameText = LOCTEXT("RenameAsset", "Rename");
		const FText RenameTooltip = LOCTEXT("RenameTooltip", "Rename selected asset");
		const FText DeleteText = LOCTEXT("DeleteAsset", "Delete");
		const FText DeleteTooltip = LOCTEXT("DeleteTooltip", "Delete selected asset");
		const FText SaveAssetText = LOCTEXT("SaveAsset", "Save");
		const FText SaveAssetTooltip = LOCTEXT("SaveAssetTooltip", "Save the selected Data Asset");
		const FText SaveAllText = LOCTEXT("SaveAll", "Save All");
		const FText SaveAllTooltip = LOCTEXT("SaveAllTooltip", "Save all modified Data Assets");
		// Assets Menu
		const FText OpenAssetText = LOCTEXT("OpenAsset", "Open Asset");
		const FText OpenAssetTooltip = LOCTEXT("OpenAssetTooltip", "Open the selected Data Asset in editor");
		const FText FindInCBText = LOCTEXT("FindInContentBrowser", "Find In CB");
		const FText FindInCBTooltip = LOCTEXT("FindInContentBrowserTooltip", "Locate asset in Content Browser");
		const FText CopyRefText = LOCTEXT("CopyReference", "Copy Reference");
		const FText CopyRefTooltip = LOCTEXT("CopyReferenceTooltip", "Copy asset reference to clipboard");
		const FText CopyPathsText = LOCTEXT("CopyPaths", "Copy Paths");
		const FText CopyPathsTooltip = LOCTEXT("CopyPathsTooltip", "Copy asset paths to clipboard");
		const FText RefViewerText = LOCTEXT("ReferenceViewer", "Reference Viewer");
		const FText RefViewerTooltip = LOCTEXT("ReferenceViewerTooltip", "Open reference viewer for this asset");
		const FText SizeMapText = LOCTEXT("SizeMap", "Size Map");
		const FText SizeMapTooltip = LOCTEXT("SizeMapTooltip", "View asset size information");
		const FText AuditAssetText = LOCTEXT("AuditAsset", "Audit Asset");
		const FText AuditAssetTooltip = LOCTEXT("AuditAssetTooltip", "Audit asset metadata");
		const FText RevisionControlText = LOCTEXT("RevisionControl", "Revision Control");
		const FText RevisionControlTooltip = LOCTEXT("RevisionControlTooltip", "Open revision control menu");
		const FText ShowAssetMetadataText = LOCTEXT("ShowAssetMetaData", "Show Asset Metadata");
		const FText ShowAssetMetadataTooltip = LOCTEXT("ShowAssetMetadataTooltip", "Display the metadata information of the selected asset.");

		// Settings Menu
		const FText DebugSectionText = LOCTEXT("DebugSection", "Debug");
		const FText OpenMessageLogText = LOCTEXT("OpenMessageLog_Label", "Open Message Log");
		const FText OpenMessageLogTooltip = LOCTEXT("OpenMessageLog_Tooltip", "Opens the Message Log window");
		const FText OpenOutputLogText = LOCTEXT("OpenOutputLog_Label", "Open Output Log");
		const FText OpenOutputLogTooltip = LOCTEXT("OpenOutputLog_Tooltip", "Opens the Output Log window");
		const FText SettingsSectionText = LOCTEXT("SettingsSection", "Settings");
		const FText ShowAssetsListText = LOCTEXT("ShowAssetsList", "Show Assets List");
		const FText ShowAssetsListTooltip = LOCTEXT("ShowAssetsListTooltip", "Toggle assets list visibility");
		const FText PluginSettingsSectionText = LOCTEXT("PluginSettingsSection", "Plugin");
		const FText PluginSettingsText = LOCTEXT("PluginSettings", "Plugin Settings");
		const FText PluginSettingsTooltip = LOCTEXT("PluginSettingsTooltip", "Open plugin settings");
		const FText RestartSectionText = LOCTEXT("RestartSection", "Maintenance");
		const FText RestartPluginText = LOCTEXT("RestartPlugin", "Restart Plugin");
		const FText RestartPluginTooltip = LOCTEXT("RestartPluginTooltip", "Restart the plugin");

		// Help Menu
		const FText DocumentationText = LOCTEXT("Documentation", "Documentation");
		const FText DocumentationTooltip = LOCTEXT("DocumentationTooltip", "Open documentation");

		// Menu Bar
		const FText FileMenuText = LOCTEXT("FileMenu", "File");
		const FText FileMenuTooltip = LOCTEXT("FileMenuTooltip", "File operations");
		const FText AssetMenuText = LOCTEXT("AssetMenu", "Asset");
		const FText AssetMenuTooltip = LOCTEXT("AssetMenuTooltip", "Asset operations");
		const FText SettingsMenuText = LOCTEXT("SettingsMenu", "Settings");
		const FText SettingsMenuTooltip = LOCTEXT("SettingsMenuTooltip", "Plugin settings");
		const FText HelpMenuText = LOCTEXT("HelpMenu", "Help");
		const FText HelpMenuTooltip = LOCTEXT("HelpMenuTooltip", "Help and documentation");
	}
}



void FDataAssetManagerMenu::FillFileMenu(FMenuBuilder& MenuBuilder, TSharedRef<IDataAssetManagerInterface> Manager)
{
	using namespace DataAssetManagerMenu;

	MenuBuilder.BeginSection(Names::ExtensionHookCreateName, Texts::CreateSectionText);
	MenuBuilder.AddMenuEntry(
		Texts::AddNewAssetText,
		Texts::AddNewAssetTooltip,
		Icons::AddNewAsset,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::CreateNewDataAsset));
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(Names::ExtensionHookEditName, Texts::EditSectionText);
	MenuBuilder.AddMenuEntry(
		Texts::SaveAssetText,
		Texts::SaveAssetTooltip,
		Icons::SaveAsset,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::SaveDataAsset));

	MenuBuilder.AddMenuEntry(
		Texts::SaveAllText, 
		Texts::SaveAllTooltip, 
		Icons::SaveAll,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::SaveAllData));

	MenuBuilder.AddMenuEntry(
		Texts::RenameText,
		Texts::RenameTooltip,
		Icons::Rename,
		CREATE_ACTION_WITH_CAN_EXECUTE(Manager, &IDataAssetManagerInterface::FocusOnSelectedAsset, 
			&IDataAssetManagerInterface::CanRename));

	MenuBuilder.AddMenuEntry(
		Texts::DeleteText,
		Texts::DeleteTooltip,
		Icons::Delete,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::DeleteDataAsset));
	MenuBuilder.EndSection();
}

void FDataAssetManagerMenu::FillAssetsMenu(FMenuBuilder& MenuBuilder, TSharedRef<IDataAssetManagerInterface> Manager)
{
	using namespace DataAssetManagerMenu;

	MenuBuilder.AddMenuEntry(
		Texts::OpenAssetText,
		Texts::OpenAssetTooltip,
		Icons::OpenAsset,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::OpenSelectedDataAssetInEditor));

	MenuBuilder.AddMenuEntry(
		Texts::FindInCBText,
		Texts::FindInCBTooltip,
		Icons::FindInCB,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::SyncContentBrowserToSelectedAsset));

	MenuBuilder.AddMenuEntry(
		Texts::ShowAssetMetadataText,
		Texts::ShowAssetMetadataTooltip,
		Icons::FindInCB,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::ShowAssetMetaData));

	MenuBuilder.AddMenuEntry(
		Texts::CopyRefText,
		Texts::CopyRefTooltip,
		Icons::Copy,
		CREATE_ACTION_WITH_PARAM(Manager, &IDataAssetManagerInterface::CopyToClipboard, false));

	MenuBuilder.AddMenuEntry(
		Texts::CopyPathsText,
		Texts::CopyPathsTooltip,
		Icons::Copy,
		CREATE_ACTION_WITH_PARAM(Manager, &IDataAssetManagerInterface::CopyToClipboard, true));

	MenuBuilder.AddMenuEntry(
		Texts::RefViewerText,
		Texts::RefViewerTooltip,
		Icons::ReferenceViewer,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::OpenReferenceViewer));

	MenuBuilder.AddMenuEntry(
		Texts::SizeMapText,
		Texts::SizeMapTooltip,
		Icons::SizeMap,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::OpenSizeMap));

	MenuBuilder.AddMenuEntry(
		Texts::AuditAssetText,
		Texts::AuditAssetTooltip,
		Icons::Audit,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::OpenAuditAsset));

	MenuBuilder.AddMenuEntry(
		Texts::RevisionControlText,
		Texts::RevisionControlTooltip,
		Icons::RevisionControl,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::ShowSourceControlDialog));
}

void FDataAssetManagerMenu::FillSettingsMenu(FMenuBuilder& MenuBuilder, TSharedRef<IDataAssetManagerInterface> Manager)
{
	using namespace DataAssetManagerMenu;

	MenuBuilder.BeginSection(Names::ExtensionHookDebugName, Texts::DebugSectionText);
	MenuBuilder.AddMenuEntry(
		Texts::OpenMessageLogText,
		Texts::OpenMessageLogTooltip,
		Icons::MessageLog,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::OpenMessageLogWindow));

	MenuBuilder.AddMenuEntry(
		Texts::OpenOutputLogText,
		Texts::OpenOutputLogTooltip,
		Icons::OutputLog,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::OpenOutputLogWindow));
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(Names::ExtensionHookSettingsName, Texts::SettingsSectionText);
	MenuBuilder.AddMenuEntry(
		Texts::ShowAssetsListText,
		Texts::ShowAssetsListTooltip,
		Icons::Visibility,
		CREATE_ACTION(Manager,&IDataAssetManagerInterface::ToggleDataAssetListVisibility));
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(Names::ExtensionHookPluginSettingsName, Texts::PluginSettingsSectionText);
	MenuBuilder.AddMenuEntry(
		Texts::PluginSettingsText,
		Texts::PluginSettingsTooltip,
		Icons::Settings,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::OpenPluginSettings));
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(Names::ExtensionHookRestartName, Texts::RestartSectionText);
	MenuBuilder.AddMenuEntry(
		Texts::RestartPluginText,
		Texts::RestartPluginTooltip,
		Icons::Refresh,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::RestartPlugin));
	MenuBuilder.EndSection();
}

void FDataAssetManagerMenu::FillHelpMenu(FMenuBuilder& MenuBuilder, TSharedRef<IDataAssetManagerInterface> Manager)
{
	using namespace DataAssetManagerMenu;
	MenuBuilder.AddMenuEntry(
		Texts::DocumentationText,
		Texts::DocumentationTooltip,
		Icons::Documentation,
		CREATE_ACTION(Manager, &IDataAssetManagerInterface::ShowDocumentation));
}

TSharedRef<SWidget> FDataAssetManagerMenuFactory::CreateMenuBar(TSharedRef<IDataAssetManagerInterface> Manager)
{
	using namespace DataAssetManagerMenu;

	FMenuBarBuilder MenuBuilder(NULL);
	MenuBuilder.AddPullDownMenu(
		Texts::FileMenuText,
		Texts::FileMenuTooltip,
		CREATE_DELEGATE_ACTION(Manager , &FDataAssetManagerMenu::FillFileMenu));

	MenuBuilder.AddPullDownMenu(
		Texts::AssetMenuText,
		Texts::AssetMenuTooltip,
		CREATE_DELEGATE_ACTION(Manager, &FDataAssetManagerMenu::FillAssetsMenu));

	MenuBuilder.AddPullDownMenu(
		Texts::SettingsMenuText,
		Texts::SettingsMenuTooltip,
		CREATE_DELEGATE_ACTION(Manager, &FDataAssetManagerMenu::FillSettingsMenu));

	MenuBuilder.AddPullDownMenu(
		Texts::HelpMenuText,
		Texts::HelpMenuTooltip,
		CREATE_DELEGATE_ACTION(Manager, &FDataAssetManagerMenu::FillHelpMenu));

	return MenuBuilder.MakeWidget();
}


#undef LOCTEXT_NAMESPACE
