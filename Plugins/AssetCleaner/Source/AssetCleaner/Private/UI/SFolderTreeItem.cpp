// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SFolderTreeItem.h"

void SFolderItemTree::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InTable)
{
	Item = InArgs._Item;
	HighlightText = InArgs._HightlightText;

	SMultiColumnTableRow::Construct(SMultiColumnTableRow::FArguments().Padding(FMargin{ 0.0f, 2.0f, 0.0f, 0.0f }), InTable);
}

TSharedRef<SWidget> SFolderItemTree::GenerateWidgetForColumn(const FName& InColumnName) 
{
	if(InColumnName.IsEqual(TEXT("Path")))
	{
		return
			SNew(SHorizontalBox)
			.ToolTipText(FText::FromString(Item->FolderPath))

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin{ 2.0f })
			[
				SNew(SExpanderArrow, SharedThis(this))
					.IndentAmount(10)
					.ShouldDrawWires(false)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 2, 0)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(GetFolderIcon())
				.ColorAndOpacity(GetFolderColor())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin{ 2.0f })
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->FolderName))
				.HighlightText(HighlightText)
			];
	}

	if(InColumnName.IsEqual(TEXT("NumAssetsTotal")))
	{
		return
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.AutoWrapText(false)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FLinearColor::White)
				.Text(FText::AsNumber(Item->NumAssetsTotal))
			];
	}

	if(InColumnName.IsEqual(TEXT("NumAssetsUsed")))
	{
		return
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.AutoWrapText(false)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FLinearColor::White)
				.Text(FText::AsNumber(Item->NumAssetsUsed))
			];
	}

	if(InColumnName.IsEqual(TEXT("NumAssetsUnused")))
	{
		return
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.AutoWrapText(false)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FLinearColor::White)
				.Text(FText::AsNumber(Item->NumAssetsUnused))
			];
	}

	if(InColumnName.IsEqual(TEXT("UnusedPercent")))
	{
		FNumberFormattingOptions FormatOptions;
		FormatOptions.UseGrouping = true;
		FormatOptions.MinimumFractionalDigits = 2;
		FormatOptions.MaximumFractionalDigits = 2;

		const FString UnusedStrPercent = FText::AsNumber(Item->PercentageUnused, &FormatOptions).ToString() + TEXT(" %");

		return
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(FMargin{ 5.0f, 1.0f })
			.FillWidth(1.0f)
			[
				SNew(SOverlay)

				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SProgressBar)
					.BorderPadding(FVector2D{ 0.0f, 0.0f })
					.Percent(Item->PercentageUnusedNormalized)
					.Style(&FAppStyle::Get().GetWidgetStyle<FProgressBarStyle>("ProgressBar"))
					
					// TODO !!! 	
					//.FillColorAndOpacity(FAppStyle::Get().GetSlateColor("Error")) 
					//.BackgroundImage(FAppStyle::Get().GetBrush("ProgressBar.Background"))
					//.FillImage(FAppStyle::Get().GetBrush("ProgressBar.Fill"))
					//.FillColorAndOpacity(FAppStyle::Get().GetSlateColor("Error"))
				]

				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.AutoWrapText(false)
					.ColorAndOpacity(FLinearColor::White)
					.Text(FText::FromString(UnusedStrPercent))
				]
			];
	}

	if(InColumnName.IsEqual(TEXT("UnusedSize")))
	{
		return
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(FMargin{ 5.0f, 1.0f }).FillWidth(1.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(false)
				.ColorAndOpacity(FLinearColor::White)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FLinearColor::White)
				.Text(FText::AsMemory(Item->SizeAssetsUnused, IEC))
			];
	}

	return SNew(STextBlock)
		.Text(FText::FromString(TEXT("")));
}


const FSlateBrush* SFolderItemTree::GetFolderIcon() const
{
	return FAppStyle::GetBrush(Item->bIsExpanded ? TEXT("ContentBrowser.AssetTreeFolderOpen") : TEXT("ContentBrowser.AssetTreeFolderClosed"));
}

FSlateColor SFolderItemTree::GetFolderColor() const
{
	if(Item->bIsExcluded)
	{
		return FAppStyle::Get().GetSlateColor("Warning");
	}

	if(Item->bIsDev)
	{
		return FAppStyle::Get().GetSlateColor("AccentBlue");
	}

	if(Item->bIsEmpty)
	{
		return FAppStyle::Get().GetSlateColor("Error");
	}

	return FAppStyle::Get().GetSlateColor("AccentBlue");
}
