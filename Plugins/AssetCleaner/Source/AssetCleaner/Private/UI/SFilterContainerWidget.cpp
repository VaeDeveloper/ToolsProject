// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SFilterContainerWidget.h"


void SFilterContainerWidget::Construct(const FArguments& InArgs)
{
	FilterList = InArgs._FilterList;
	OnFilterChangedDelegate = InArgs._OnFilterChanged;
	ChildSlot
		[
			SAssignNew(ScrollBox, SScrollBox)
		];

	RebuildFilters();
}

void SFilterContainerWidget::FCustomFilter::Construct(const FArguments& InArgs)
{
	OnFilterChanged = InArgs._OnFilterChanged;
	ToolTipText = InArgs._ToolTipText;
	Construct_Internal(InArgs._FilterName);
}

void SFilterContainerWidget::FCustomFilter::Construct_Internal(const FString& FilterName)
{
	FilterDispayName = FilterName;

	TSharedPtr<SWidget> ContentWidget =
		SNew(SBorder)
		.Padding(1.0f)
		.BorderImage(FAppStyle::Get().GetBrush("FilterBar.FilterBackground"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("FilterBar.FilterImage"))
				.ColorAndOpacity(this, &FCustomFilter::GetFilterImageColorAndOpacity)
			]

				+ SHorizontalBox::Slot()
				.Padding(TAttribute<FMargin>(this, &FCustomFilter::GetFilterNamePadding))
				.VAlign(VAlign_Center)
				[
					SAssignNew(ToggleButtonPtr, SCheckBox)
					.Style(FAppStyle::Get(), "FilterBar.FilterButton")
					.CheckBoxContentUsesAutoWidth(false)
					.ToolTipText(FText::FromString(ToolTipText))
					.IsChecked(this, &FCustomFilter::IsChecked)
					.OnCheckStateChanged(this, &FCustomFilter::FilterToggled)
					.OnGetMenuContent(this, &FCustomFilter::GetRightClickMenuContent)
						[
							SNew(STextBlock)
							.Text(FText::FromString(GetFilterDisplayName()))
							.IsEnabled_Lambda([this] { return bEnabled; })
						]
				]
		];

	ChildSlot
		[
			ContentWidget.ToSharedRef()
		];
}

void SFilterContainerWidget::RebuildFilters()
{
	if(!ScrollBox.IsValid()) return;

	ScrollBox->ClearChildren();
	FilterWidgets.Empty();

	for(const FNamedFilterData& FilterData : FilterList)
	{
		TSharedPtr<FCustomFilter> NewFilter = 
			SNew(FCustomFilter)
			.FilterName(FilterData.FilterName)
			.ToolTipText(FilterData.ToolTipText)
			.OnFilterChanged(OnFilterChangedDelegate);

		ScrollBox->AddSlot()
			.Padding(2.0f)
			[
				NewFilter.ToSharedRef()
			];

		FilterWidgets.Add(NewFilter);
	}
}
