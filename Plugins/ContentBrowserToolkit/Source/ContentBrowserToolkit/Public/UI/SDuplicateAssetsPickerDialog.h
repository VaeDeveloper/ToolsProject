// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectTools.h"
#include "Widgets/SCompoundWidget.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Widgets/Input/SHyperlink.h"  

struct FDuplicateAssetInfo
{
	FString AssetName;
	TArray<FAssetData> Assets;
};

class CONTENTBROWSERTOOLKIT_API SDuplicateAssetsPickerDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDuplicateAssetsPickerDialog) {}
		SLATE_ARGUMENT(TArray<TSharedPtr<FDuplicateAssetInfo>>, DuplicateAssets)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		DuplicateAssets = InArgs._DuplicateAssets;

		TSharedRef<SVerticalBox> VerticalBox =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("These are the duplicate assets found:")))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SListView<TSharedPtr<FDuplicateAssetInfo>>)
					.ItemHeight(24)
					.ListItemsSource(&DuplicateAssets)
					.OnGenerateRow(this, &SDuplicateAssetsPickerDialog::GenerateRowForDuplicateAsset)
					.HeaderRow(
						SNew(SHeaderRow)
						+ SHeaderRow::Column("AssetName")
						.DefaultLabel(FText::FromString("Name"))
						.FillWidth(0.3f)

						+ SHeaderRow::Column("AssetPaths")
						.DefaultLabel(FText::FromString("Path(s)"))
						.FillWidth(0.7f)
					)
			];

		ChildSlot
			[
				VerticalBox
			];
	}

private:
	TSharedRef<ITableRow> GenerateRowForDuplicateAsset(TSharedPtr<FDuplicateAssetInfo> InItem, const TSharedRef<STableViewBase>& OwnerTable)
	{
		return SNew(STableRow<TSharedPtr<FDuplicateAssetInfo>>, OwnerTable)
			[
				SNew(SHorizontalBox)

				// Column: Name
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InItem->AssetName))
				]

				// Column: Path(s) Ч список ссылок
				+ SHorizontalBox::Slot()
				.FillWidth(0.7f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						GenerateClickableAssetList(InItem->Assets)
					]
				]
			];
	}

	TSharedRef<SWidget> GenerateClickableAssetList(const TArray<FAssetData>& Assets)
	{
		TSharedRef<SVerticalBox> VBox = SNew(SVerticalBox);

		for(const FAssetData& Asset : Assets)
		{
			VBox->AddSlot()
				.AutoHeight()
				.Padding(0, 2)
				[
					SNew(SHyperlink)
					.Style(&FAppStyle::Get(), "HoverOnlyHyperlink")
					.Text(FText::FromString(Asset.PackageName.ToString()))
					.Cursor(EMouseCursor::Hand)
					.OnNavigate_Lambda([Asset] ()
						{
							if(Asset.IsValid())
							{
								const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
								TArray<FAssetData> ToFocus = { Asset };
								ContentBrowserModule.Get().SyncBrowserToAssets(ToFocus);
							}
						})
				];
		}

		return VBox;
	}

	TArray<TSharedPtr<FDuplicateAssetInfo>> DuplicateAssets;
	TMap<TSharedPtr<FDuplicateAssetInfo>, int32> FocusedAssetIndices;

};