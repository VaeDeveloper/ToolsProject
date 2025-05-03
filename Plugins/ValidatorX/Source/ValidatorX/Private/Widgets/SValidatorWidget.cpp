// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SValidatorWidget.h"
#include "BaseClasses/BlueprintValidatorBase.h"
#include "Styling/SlateStyleRegistry.h"

namespace ValidatorListColumns
{
	static const FName ColumnID_Type("Type");
	static const FName ColumnID_Name("Name");
	static const FName ColumnID_Button("Button");
}

FString AddSpacesBeforeUppercase(const FString& Input)
{
	FString Result;
	Result.Reserve(Input.Len() * 2); 

	for(int32 i = 0; i < Input.Len(); ++i)
	{
		const TCHAR Char = Input[i];
		if(i > 0 && FChar::IsUpper(Char) && !FChar::IsWhitespace(Input[i - 1]))
		{
			Result += TEXT(" ");
		}
		Result += Char;
	}

	return Result;
}

class SValidatorTableRow : public SMultiColumnTableRow<TWeakObjectPtr<UBlueprintValidatorBase>>
{
public:
	SLATE_BEGIN_ARGS(SValidatorTableRow) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UBlueprintValidatorBase>, Validator)
		SLATE_ARGUMENT(FSlateFontInfo, Font)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Validator = InArgs._Validator;
		LocalFont = InArgs._Font;

		SMultiColumnTableRow::Construct(FSuperRowType::FArguments()
			.Style(FAppStyle::Get(), "ContentBrowser.AssetListView.ColumnListTableRow"), InOwnerTable);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnId) override
	{
		if (ColumnId == ValidatorListColumns::ColumnID_Type)
		{
			FString Type;
			if(Validator.IsValid())
			{
				Type = Validator->GetTypeValidator();
			}

			TSharedRef<SBox> TypeBox = SNew(SBox)
				.Padding(4.0f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Type))
						.Font(LocalFont).Justification(ETextJustify::Center)
				];

			return TypeBox;
		}
		else if (ColumnId == ValidatorListColumns::ColumnID_Name)
		{
			FString CleanName = Validator->GetName();
			int32 UnderscoreIndex;
			if(CleanName.FindLastChar('_', UnderscoreIndex))
			{
				const FString Suffix = CleanName.Mid(UnderscoreIndex + 1);
				if(Suffix.IsNumeric())
				{
					CleanName = CleanName.Left(UnderscoreIndex);
				}
			}

			CleanName = AddSpacesBeforeUppercase(CleanName);
			TSharedRef<SBox> NameBox = SNew(SBox)
				.Padding(4.0f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString(CleanName))
						.Font(LocalFont).Justification(ETextJustify::Center)
				];

			return NameBox;
		}
		else if (ColumnId == ValidatorListColumns::ColumnID_Button)
		{
			
			return SNew(SBox)
				.Padding(4.0f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(SCheckBox)
						.IsChecked_Lambda([this] 
							{ 
								if(Validator.IsValid())
								{
									return Validator->IsEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
								}
								return ECheckBoxState::Unchecked;
							})
						.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
							{
								if(Validator.IsValid())
								{
									switch(NewState)
									{
									case ECheckBoxState::Unchecked:
										Validator->SetValidationEnabled(false);
										UE_LOG(LogTemp, Warning, TEXT("Validator %s Disabled (IsEnabled: %s)"),
										*Validator->GetName(),
										Validator->IsEnabled() ? TEXT("true") : TEXT("false"));
										break;

									case ECheckBoxState::Checked:
										Validator->SetValidationEnabled(true);
										UE_LOG(LogTemp, Warning, TEXT("Validator %s Enabled (IsEnabled: %s)"),
										*Validator->GetName(), 
										Validator->IsEnabled() ? TEXT("true") : TEXT("false"));
										break;

									default:
										break;
									}
								}
							})
				];
		}

		return SNullWidget::NullWidget;
	}


	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		return FReply::Unhandled();
	}

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
	{
		return FReply::Unhandled();
	}

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		return FReply::Unhandled();
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		return FReply::Unhandled();
	}

private:
	TWeakObjectPtr<UBlueprintValidatorBase> Validator;
	FSlateFontInfo LocalFont;
};

void SValidatorWidget::Construct(const FArguments& InArgs)
{
	LocalValidators = InArgs._Validators;

	FontInfo = FAppStyle::GetFontStyle("NormalFont");
	FontInfo.Size = 15.0f;
	
	TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator).Thickness(1.0f)
		]

		+ SVerticalBox::Slot()
		.Padding(4)
		[
			SNew(SExpandableArea)
				.InitiallyCollapsed(false)
				.AreaTitle(FText::FromString("Blueprint Validators"))
				.AreaTitleFont(FontInfo)
				.BodyContent()
				[
					SAssignNew(ListViewWidget, SListView<TWeakObjectPtr<UBlueprintValidatorBase>>)
						.ListItemsSource(&LocalValidators)
						.OnGenerateRow(this, &SValidatorWidget::OnGenerateRowForList)
						.SelectionMode(ESelectionMode::None)
						.HeaderRow
						(
							SNew(SHeaderRow)
							+ SHeaderRow::Column(ValidatorListColumns::ColumnID_Type)
							.FillWidth(0.4f)
							.FixedWidth(StaticCast<TOptional<float>>(200.0f))
							[
								SNew(STextBlock).Text(FText::FromString("Type")).Justification(ETextJustify::Center).Font(FontInfo)
							]

							+ SHeaderRow::Column(ValidatorListColumns::ColumnID_Name)
							.FillWidth(0.4f)
							[
								SNew(STextBlock).Text(FText::FromString("Validator Name")).Justification(ETextJustify::Center).Font(FontInfo)
							]

							+ SHeaderRow::Column(ValidatorListColumns::ColumnID_Button)
							.FixedWidth(StaticCast<TOptional<float>>(50.0f))
							[
								SNew(SBox)
									.VAlign(VAlign_Center)
									.HAlign(HAlign_Center)
									[
										SNew(SCheckBox)
											.HAlign(HAlign_Center)
											.ToolTipText(FText::FromString("Enable/Disable all validators"))
											.OnCheckStateChanged_Lambda([this] (ECheckBoxState NewState)
												{
													const bool bEnableAll = (NewState == ECheckBoxState::Checked);
													for(const TWeakObjectPtr<UBlueprintValidatorBase>& Validator : LocalValidators)
													{
														if(Validator.IsValid())
														{
															Validator->SetValidationEnabled(bEnableAll);
														}
													}
													if(ListViewWidget.IsValid())
													{
														ListViewWidget->RequestListRefresh();
													}
												})

									]
							]
						)
				]
		];

	ChildSlot
	[
		VerticalBox
	];
}

TSharedRef<ITableRow> SValidatorWidget::OnGenerateRowForList(TWeakObjectPtr<UBlueprintValidatorBase> InItem,const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SValidatorTableRow, OwnerTable)
		.Validator(InItem)
		.Font(FontInfo);
}