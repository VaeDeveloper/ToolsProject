// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMultiLineEditableText;
/**
 * 
 */
class UNOTEPAD_API SNotepadWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNotepadWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);


private:

	FString DocumentPathText;
	FString DocumentContentText;
	FString DocumentName;
	FString FinalDocumentPath;

	TSharedPtr<SMultiLineEditableText> DocumentContentTextBox;


	/**
	 * Menu bar widget.
	 *
	 * A shared pointer to the widget representing the menu bar in the user interface.
	 */
	TSharedPtr<SWidget> MenuBar;

	void FillFileMenu(FMenuBuilder& MenuBuilder);
	void FillSettingsMenu(FMenuBuilder& MenuBuilder);

	FString LastSavedFilePath;
	void OpenTextFileDialog();
	void CreateNewDocument();
	TSharedRef<SWidget> GetDocumentContent();
	void SaveCurrentDocument();
	void SaveAsDocumentInFile(bool bSaveAs);
	FSlateFontInfo FontInfo;
	FTextBlockStyle DocumentContentTextStyle;
	TSharedPtr<FTabManager> TabManager;
	TSharedPtr<FTabManager::FLayout> TabLayout;
};
