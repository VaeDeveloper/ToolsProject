// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SDeveloperSettingsWidget.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DeveloperSettings/DataAssetManagerSettings.h"
#include "DetailWidgetRow.h"
#include "DataAssetManager.h"
#include "EditorValidatorSubsystem.h"

TSharedRef<IDetailCustomization> SDeveloperSettingsWidget::MakeInstance()
{
	return MakeShareable(new SDeveloperSettingsWidget);
}

void SDeveloperSettingsWidget::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;
	DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
	if (SelectedObjects.Num() == 0) return;

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Settings");
	Category.AddCustomRow(FText::FromString("Settings"))
	.WholeRowContent()
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Restart Plugin "))

			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(-440, 0, 0, 0))
			[
				SNew(SBox)
				.WidthOverride(220.0f)
				[
					SNew(SButton)
					.Text(FText::FromString("Restart Plugin"))
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.OnClicked_Lambda([]()
					{
						FDataAssetManagerModule& Module = FModuleManager::LoadModuleChecked<FDataAssetManagerModule>(("DataAssetManager"));
						Module.RestartWidget();
						return FReply::Handled();
					})
				]
			]
		]
	];
}