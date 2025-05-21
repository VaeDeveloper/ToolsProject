// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SAssetCleanerTableRow.h"


void SAssetCleanerTableRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
{
	Item = InArgs._Item;
	AddDirtyEventHandler(Item->PackageName.ToString());

	OnAssetRenamed = InArgs._OnAssetRenamed;
	OnCreateContextMenu = InArgs._OnCreateContextMenu;
	OnAssetDoubleClicked = InArgs._OnAssetDoubleClicked;
	OnRegisterEditableText = InArgs._OnRegisterEditableText;
	MouseButtonDown = InArgs._OnMouseButtonDown;

	SMultiColumnTableRow::Construct(FSuperRowType::FArguments()
		.Style(FAppStyle::Get(), "ContentBrowser.AssetListView.ColumnListTableRow"), InOwnerTable);
}

SAssetCleanerTableRow::~SAssetCleanerTableRow()
{
	if(OnPackageDirtyStateChangedHandle.IsValid())
	{
		UPackage::PackageDirtyStateChangedEvent.Remove(OnPackageDirtyStateChangedHandle);
	}
}

TSharedRef<SWidget> SAssetCleanerTableRow::GenerateWidgetForColumn(const FName& ColumnId)
{
	if(ColumnId == AssetCleanerListColumns::ColumnID_Name)
	{
		TSharedRef<SEditableText> EditableText =
			SNew(SEditableText)
			.Cursor(EMouseCursor::Hand)
			.HintText(FText::FromName(Item->PackagePath))
			.Text_Lambda([this] () { return FText::FromName(Item->AssetName); })
			.SelectAllTextWhenFocused(true)
			.OnTextCommitted_Lambda([this] (const FText& Text, ETextCommit::Type Type) {
			if(OnAssetRenamed.IsBound() && Type == ETextCommit::OnEnter)
			{
				OnAssetRenamed.Execute(Item, Text, Type);
			}
				});

		if(OnRegisterEditableText.IsBound())
		{
			OnRegisterEditableText.Execute(Item, EditableText);

		}

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("ContentBrowser.ColumnViewAssetIcon"))
					.ColorAndOpacity(FColor::FromHex("616161FF"))
				]

				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Bottom)
				[
					SAssignNew(DirtyBrushWidget, SImage)
					.Image(FAppStyle::GetBrush("Icons.DirtyBadge"))
					.Visibility(EVisibility::Collapsed)
				]
			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.VAlign(VAlign_Center)
				.Padding(6.0f, 0.0f, 0.0f, 0.0f)
				[
					EditableText
				]

				+ SOverlay::Slot()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.Visibility(EVisibility::Visible)
				]

				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.Cursor(EMouseCursor::Hand)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.ColorAndOpacity(FColor::Transparent)
					.BorderBackgroundColor(FColor::Transparent)
					.OnMouseButtonDown(this, &SAssetCleanerTableRow::BorderMouseButtonDown)
					.OnMouseDoubleClick(this, &SAssetCleanerTableRow::BorderMouseDoubleClicked)
				]
			];
	}
	else if(ColumnId == AssetCleanerListColumns::ColumnID_Type)
	{
		return SNew(STextBlock) // bug fix in 5.5 version GetClass() returned nullptr on some asset classes
			.Text(FText::FromName(Item.IsValid() ? Item->AssetClassPath.GetAssetName() : NAME_None));
	}
	else if(ColumnId == AssetCleanerListColumns::ColumnID_DiskSize)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(AssetCleaner::Private::GetAssetDiskSize(Item.ToSharedRef().Get())));
	}
	else if(ColumnId == AssetCleanerListColumns::ColumnID_Path)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(Item->PackageName.ToString()));
	}
	else if(ColumnId == AssetCleanerListColumns::ColumnID_RC)
	{
		const FString AssetPath = FPackageName::LongPackageNameToFilename(Item->PackageName.ToString(), FPackageName::GetAssetPackageExtension());
		FSourceControlStatePtr SourceControlState = ISourceControlModule::Get().GetProvider().GetState(AssetPath, EStateCacheUsage::Use);
		const FSlateBrush* IconBrush = FAppStyle::GetBrush("SourceControl.Generic");
		//TODO !!!
		if(SourceControlState.IsValid())
		{
			if(SourceControlState->IsCheckedOut())
			{
				IconBrush = FAppStyle::GetBrush("SourceControl.CheckedOut");
			}
			else if(SourceControlState->IsModified())
			{
				IconBrush = FAppStyle::GetBrush("SourceControl.Modified");
			}
			else if(SourceControlState->IsSourceControlled())
			{
				IconBrush = FAppStyle::GetBrush("SourceControl.CheckedIn");
			}
			else
			{
				IconBrush = FAppStyle::GetBrush("SourceControl.NotUnderSourceControl");
			}
		}

		return SNew(SImage)
			.Image(IconBrush)
			.ColorAndOpacity(FColor::Transparent);
	}

	return SNullWidget::NullWidget;
}

void SAssetCleanerTableRow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if(DirtyBrushWidget.IsValid())
	{
		DirtyBrushWidget->SetVisibility(bIsDirty ? EVisibility::Visible : EVisibility::Collapsed);
	}
}

FReply SAssetCleanerTableRow::BorderMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
	if(OnCreateContextMenu.IsBound() && MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		OnCreateContextMenu.Execute(InGeometry, MouseEvent);
		return FReply::Handled();
	}

	if(MouseButtonDown.IsBound())
	{
		return MouseButtonDown.Execute(InGeometry, MouseEvent);
	}

	return FReply::Unhandled();
}

FReply SAssetCleanerTableRow::BorderMouseDoubleClicked(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
	if(OnAssetDoubleClicked.IsBound())
	{
		OnAssetDoubleClicked.Execute(InGeometry, MouseEvent);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SAssetCleanerTableRow::AddDirtyEventHandler(const FString& PackageName)
{
	if(Item.IsValid())
	{
		OnPackageDirtyStateChangedHandle = UPackage::PackageDirtyStateChangedEvent.AddLambda(
				[this, PackageName] (UPackage* DirtyPackage)
				{
					if(DirtyPackage && DirtyPackage->GetName() == PackageName)
					{
						bIsDirty = DirtyPackage->IsDirty();
					}
				});
	}
}

