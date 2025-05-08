// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SNotepadWidget.h"
#include "DesktopPlatformModule.h"
#include "Widgets/Text/SMultiLineEditableText.h"

#define LOCTEXT_NAMESPACE "UNotepadWidget"

namespace SlateStatic
{
	static TSharedRef<SSeparator> CreateTransparentSeparator(float ThicknessValue)
	{
		return SNew(SSeparator)
			.Orientation(Orient_Vertical)
			.Thickness(ThicknessValue)
			.ColorAndOpacity(FColor::Transparent);
	}

	void FindAllTxtFiles(const FString& DirectoryPath, TArray<TSharedPtr<FString>>& FileItemList)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		if(PlatformFile.DirectoryExists(*DirectoryPath))
		{
			TArray<FString> FoundFiles;
			PlatformFile.FindFilesRecursively(FoundFiles, *DirectoryPath, TEXT(".txt"));

			for(const FString& FilePath : FoundFiles)
			{
				FileItemList.Add(MakeShared<FString>(FilePath));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Directory does not exist: %s"), *DirectoryPath);
		}
	}
}

void SNotepadWidget::Construct(const FArguments& InArgs)
{
	FString DirectoryName = "Documents";
	DocumentPathText = FPaths::ProjectDir() / DirectoryName;

	FontInfo = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	FontInfo.Size = 15.0f;
	DocumentContentTextStyle = FTextBlockStyle().SetFont(FontInfo).SetColorAndOpacity(FSlateColor(FLinearColor::Black));

	FMenuBarBuilder MenuBuilder(NULL);
	MenuBuilder.AddPullDownMenu(
		LOCTEXT("FileMenu", "File"),
		LOCTEXT("FileMenu_ToolTip", "Open the file menu"),
		FNewMenuDelegate::CreateSP(this, &SNotepadWidget::FillFileMenu));
	
	MenuBuilder.AddPullDownMenu(
		LOCTEXT("SettingsMenu", "Settings"),
		LOCTEXT("SettingsMenu_ToolTip", "Open the settings menu"),
		FNewMenuDelegate::CreateSP(this, &SNotepadWidget::FillSettingsMenu));
	TSharedRef<SWidget> MenuBarWidget = MenuBuilder.MakeWidget();

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
					.Size(FVector2D(0.0f, 6.0f))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MenuBarWidget
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
					.Size(FVector2D(0.0f, 6.0f))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
					.Thickness(2.0f)
					.ColorAndOpacity(FColor::White)
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(20.0f)
			[
				SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor::Gray))
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					.ColorAndOpacity(FLinearColor::Gray)
					.ForegroundColor(FSlateColor(FLinearColor::Gray))
					[
						SNew(SBox)
							.VAlign(VAlign_Fill)
							.HAlign(HAlign_Fill)
							[
								GetDocumentContent()
							]
					]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SlateStatic::CreateTransparentSeparator(1.0f)
			]
		];
}


TSharedRef<SWidget> SNotepadWidget::GetDocumentContent()
{
	TSharedRef<SMultiLineEditableText> FinalContextTextBox =
		SNew(SMultiLineEditableText)
		.Text(FText::FromString(DocumentContentText))
		.TextStyle(&DocumentContentTextStyle)
		.Font_Lambda([&] ()
			{
				FSlateFontInfo TextFontInfo = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
				TextFontInfo.Size = 12;
				return TextFontInfo;
			})
		.HintText(FText::FromString(TEXT("Write some text here ... ")))
		.OnTextChanged_Lambda([this] (const FText& NewText)
			{
				DocumentContentText = NewText.ToString();
			})
		.OnTextCommitted_Lambda([this] (const FText& NewText, ETextCommit::Type)
			{
				DocumentContentText = NewText.ToString();
			});

	DocumentContentTextBox = FinalContextTextBox;

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			FinalContextTextBox
		];
}

void SNotepadWidget::SaveAsDocumentInFile(bool bSaveAs)
{
	if(!DocumentContentTextBox.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Text editor is not initialized"));
		return;
	}

	const FString CurrentText = DocumentContentTextBox->GetText().ToString();
	FString SavePath = LastSavedFilePath.IsEmpty() ? DocumentPathText : LastSavedFilePath;

	if(bSaveAs || SavePath.IsEmpty())
	{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		if(!DesktopPlatform)
		{
			UE_LOG(LogTemp, Error, TEXT("DesktopPlatform module is not available"));
			return;
		}

		TArray<FString> OutFiles;
		const FString DefaultPath = FPaths::ProjectDir();
		const FString FileTypes = TEXT("Text Files (*.txt)|*.txt|All Files (*.*)|*.*");

		const bool bFileSelected = DesktopPlatform->SaveFileDialog(
			nullptr,
			TEXT("Save Text File"),
			DefaultPath,
			TEXT("Untitled.txt"),
			FileTypes,
			EFileDialogFlags::None,
			OutFiles
		);

		if(!bFileSelected || OutFiles.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("Save cancelled by user"));
			return;
		}

		SavePath = OutFiles[0];
		LastSavedFilePath = SavePath;
	}

	// Разбиваем текст на строки
	TArray<FString> Lines;
	CurrentText.ParseIntoArrayLines(Lines);

#if WITH_EDITOR
	if(GIsEditor)
	{
		FScopedSlowTask SlowTask(Lines.Num(), FText::FromString(TEXT("Saving File...")));
		SlowTask.MakeDialog(true);

		FString CombinedText;
		for(int32 i = 0; i < Lines.Num(); ++i)
		{
			CombinedText += Lines[i] + LINE_TERMINATOR;
			SlowTask.EnterProgressFrame(1.f);

			// Можно добавить задержку, чтобы пользователь увидел прогресс
			// FPlatformProcess::Sleep(0.001f);
		}

		if(FFileHelper::SaveStringToFile(CombinedText, *SavePath, FFileHelper::EEncodingOptions::ForceUTF8))
		{
			FSlateApplication::Get().SetKeyboardFocus(DocumentContentTextBox);
			UE_LOG(LogTemp, Log, TEXT("File saved successfully: %s"), *SavePath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save file: %s"), *SavePath);
		}
	}
	else
#endif
	{
		if(FFileHelper::SaveStringToFile(CurrentText, *SavePath, FFileHelper::EEncodingOptions::ForceUTF8))
		{
			FSlateApplication::Get().SetKeyboardFocus(DocumentContentTextBox,EFocusCause::SetDirectly);
			UE_LOG(LogTemp, Log, TEXT("File saved successfully: %s"), *SavePath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save file: %s"), *SavePath);
		}
	}
}



void SNotepadWidget::FillFileMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("OpenFile", "New                "),
		LOCTEXT("OpenFile_ToolTip", "Create New file "),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Adjust"),
		FUIAction(FExecuteAction::CreateSP(this, &SNotepadWidget::CreateNewDocument)));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("OpenFile", "Open                "),
		LOCTEXT("OpenFile_ToolTip", "Open file "),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Adjust"),
		FUIAction(FExecuteAction::CreateSP(this, &SNotepadWidget::OpenTextFileDialog)));

	MenuBuilder.AddMenuSeparator();

	MenuBuilder.AddMenuEntry(
		LOCTEXT("Save_Entry", "Save             "),
		LOCTEXT("SaveText_ToolTip", "Save current text in file"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Adjust"),
		FUIAction(FExecuteAction::CreateLambda([this] ()
			{
				SaveAsDocumentInFile(false);
			})));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("Save_Entry", "Save As...        "),
		LOCTEXT("SaveText_ToolTip", "Save current text in file"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Adjust"),
		FUIAction(FExecuteAction::CreateLambda([this] ()
			{
				SaveAsDocumentInFile(true);
			})));

}

void SNotepadWidget::FillSettingsMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("PluginSetings", "Plugin Settngs...        "),
		LOCTEXT("PluginSettings_ToolTip", "Open the Plugin Settings"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Adjust"),
		FUIAction(FExecuteAction::CreateLambda([] () {})));
}

void SNotepadWidget::OpenTextFileDialog()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if(!DesktopPlatform) return;

	TArray<FString> OutFiles;
	FString DefaultPath = FPaths::ProjectDir();
	FString FileTypes = TEXT("Text Files (*.txt)|*.txt|All Files (*.*)|*.*");

	bool bFileSelected = DesktopPlatform->OpenFileDialog(
		nullptr,
		TEXT("Select a Text File"),
		DefaultPath,
		TEXT(""),
		FileTypes,
		EFileDialogFlags::None,
		OutFiles
	);

	if(bFileSelected && OutFiles.Num() > 0)
	{
		const FString SelectedFile = OutFiles[0];

		TArray<FString> FileLines;
		const bool bLoaded = FFileHelper::LoadFileToStringArray(FileLines, *SelectedFile);

		if(bLoaded && DocumentContentTextBox.IsValid())
		{
			const int32 TotalLines = FileLines.Num();
			FScopedSlowTask SlowTask(TotalLines, FText::FromString(TEXT("Loading File...")));
			SlowTask.MakeDialog();

			FString CombinedText;
			for(int32 i = 0; i < TotalLines; ++i)
			{
				CombinedText += FileLines[i] + LINE_TERMINATOR;
				SlowTask.EnterProgressFrame(1.f);
			}

			DocumentContentTextBox->SetText(FText::FromString(CombinedText));
			FSlateApplication::Get().SetKeyboardFocus(DocumentContentTextBox, EFocusCause::SetDirectly);
			UE_LOG(LogTemp, Log, TEXT("Document Open: %s"), *SelectedFile);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to read file: %s"), *SelectedFile);
		}
	}
}

void SNotepadWidget::CreateNewDocument()
{
	if(!DocumentContentText.IsEmpty())
	{
		EAppReturnType::Type UserChoice = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			LOCTEXT("SaveChangesPrompt", "Do you want to save the current document before creating a new one?"));

		if(UserChoice == EAppReturnType::Yes)
		{
			SaveCurrentDocument();
		}
		else if(UserChoice == EAppReturnType::Cancel)
		{
			return;
		}
	}

	DocumentContentText = TEXT("");
	if(DocumentContentTextBox.IsValid())
	{
		DocumentContentTextBox->SetText(FText::FromString(DocumentContentText));

		// Устанавливаем фокус на текстовое поле
		FSlateApplication::Get().SetKeyboardFocus(DocumentContentTextBox);
	}

	UE_LOG(LogTemp, Log, TEXT("New document created."));
}


void SNotepadWidget::SaveCurrentDocument()
{
}


#undef LOCTEXT_NAMESPACE