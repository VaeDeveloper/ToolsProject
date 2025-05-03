// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnUnusedAssetsConfirmed, const TArray<FAssetData>&);
DECLARE_DELEGATE(FOnUnusedAssetsCanceled);

/**
 * 
 */
namespace UnusedAssetPicker
{
	namespace UnusedAssetListColumn
	{
		static const FName ColumnAsset("Asset");
		static const FName ColumnAction("Action");
	}

	constexpr float BoxOverrideWidthWidget = 500.0f;
	constexpr float FixedWidthHeaderRow = 28.0f;
}

class SUnusedAssetPickerTableRow : public SMultiColumnTableRow<TSharedPtr<FAssetData>>
{
public:
	SLATE_BEGIN_ARGS(SUnusedAssetPickerTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FAssetData>, Asset)
		SLATE_ARGUMENT(TFunction<void(ECheckBoxState, FAssetData)>, OnCheckChanged)
		SLATE_ARGUMENT(TFunction<ECheckBoxState(FAssetData)>, IsChecked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Asset = InArgs._Asset;
		OnCheckChanged = InArgs._OnCheckChanged;
		IsChecked = InArgs._IsChecked;


		SMultiColumnTableRow::Construct(FSuperRowType::FArguments()
			.Style(FAppStyle::Get(), "ContentBrowser.AssetListView.ColumnListTableRow"), InOwnerTable);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnId) override
	{
		if(ColumnId == UnusedAssetPicker::UnusedAssetListColumn::ColumnAction)
		{
			return SNew(SBox).
				HAlign(HAlign_Center).
				VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([this] () { return IsChecked(*Asset); })
					.OnCheckStateChanged_Lambda([this] (ECheckBoxState NewState) { OnCheckChanged(NewState, *Asset); })
				];
		}
		else if(ColumnId == UnusedAssetPicker::UnusedAssetListColumn::ColumnAsset)
		{
			return SNew(STextBlock)
				.Text(FText::FromName(Asset.IsValid() ? Asset->PackageName : NAME_None))
				.Justification(ETextJustify::Left);
		}

		return SNullWidget::NullWidget;
	}


private:
	TSharedPtr<FAssetData> Asset;
	TFunction<void(ECheckBoxState, FAssetData)> OnCheckChanged;
	TFunction<ECheckBoxState(FAssetData)> IsChecked;
};


class SUnusedAssetPickerDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnusedAssetPickerDialog) {}
		SLATE_ARGUMENT(TArray<FAssetData>, Assets)
		SLATE_ARGUMENT(FOnUnusedAssetsConfirmed, OnConfirmed)
		SLATE_ARGUMENT(FOnUnusedAssetsCanceled, OnCanceled)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		for(const FAssetData& Asset : InArgs._Assets)
		{
			AssetList.Add(MakeShared<FAssetData>(Asset));
		}

		OnConfirmed = InArgs._OnConfirmed;
		OnCanceled = InArgs._OnCanceled;

		ChildSlot
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1)
				[
					SNew(SBox)
					.WidthOverride(UnusedAssetPicker::BoxOverrideWidthWidget)
					[
						SAssignNew(AssetListView, SListView<TSharedPtr<FAssetData>>)
						.ListItemsSource(&AssetList)
						.OnGenerateRow(this, &SUnusedAssetPickerDialog::OnGenerateRow)
						.SelectionMode(ESelectionMode::None)
						.HeaderRow
						(
							SNew(SHeaderRow)
							+ SHeaderRow::Column(UnusedAssetPicker::UnusedAssetListColumn::ColumnAction)
							.FixedWidth(StaticCast<TOptional<float>>(UnusedAssetPicker::FixedWidthHeaderRow))
							[
								SNew(SCheckBox)
								.ToolTipText(FText::FromString(TEXT("Select / Deselect All")))
								.OnCheckStateChanged(this, &SUnusedAssetPickerDialog::OnSelectAllCheckStateChanged)
							]

							+ SHeaderRow::Column(UnusedAssetPicker::UnusedAssetListColumn::ColumnAsset)
							.DefaultLabel(FText::FromString(TEXT("Asset")))
						)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("OK")))
						.OnClicked(this, &SUnusedAssetPickerDialog::OnOkClicked)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5, 0, 0, 0)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("Cancel")))
						.OnClicked(this, &SUnusedAssetPickerDialog::OnCancelClicked)
					]
				]
			];
	}

	FReply OnOkClicked()
	{
		if(OnConfirmed.IsBound())
		{
			OnConfirmed.Execute(GetSelectedAssets());
		}
		return FReply::Handled();
	}

	FReply OnCancelClicked()
	{
		if(OnCanceled.IsBound())
		{
			OnCanceled.Execute();
		}

		return FReply::Handled();
	}

	const TArray<FAssetData> GetSelectedAssets() const
	{
		TArray<FAssetData> Result = SelectedAssets.Array();
		return Result;
	}

	FOnUnusedAssetsConfirmed OnConfirmed;
	FOnUnusedAssetsCanceled OnCanceled;

private:
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetListView;
	TArray<TSharedPtr<FAssetData>> AssetList;
	TSet<FAssetData> SelectedAssets;

	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FAssetData> Item, const TSharedRef<STableViewBase>& OwnerTable)
	{
		return SNew(SUnusedAssetPickerTableRow, OwnerTable)
			.Asset(Item)
			.OnCheckChanged([this] (ECheckBoxState NewState, FAssetData Asset) { OnCheckStateChanged(NewState, Asset); })
			.IsChecked([this] (FAssetData Asset)
				{
					return SelectedAssets.Contains(Asset) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				});
	}
	void OnCheckStateChanged(ECheckBoxState NewState, FAssetData Asset)
	{
		if(NewState == ECheckBoxState::Checked)
		{
			SelectedAssets.Add(Asset);
		}
		else
		{
			SelectedAssets.Remove(Asset);
		}
	}

	void OnSelectAllCheckStateChanged(ECheckBoxState NewState)
	{
		bool bSelectAll = (NewState == ECheckBoxState::Checked);
		SelectedAssets.Empty();

		if(bSelectAll)
		{
			for(const TSharedPtr<FAssetData>& Asset : AssetList)
			{
				SelectedAssets.Add(*Asset);
			}
		}

		AssetListView->RequestListRefresh();
	}
};

