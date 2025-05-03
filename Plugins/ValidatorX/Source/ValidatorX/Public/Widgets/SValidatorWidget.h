// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UBlueprintValidatorBase;
/**
 * 
 */
class VALIDATORX_API SValidatorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SValidatorWidget) {}
		SLATE_ARGUMENT(TArray<TWeakObjectPtr<UBlueprintValidatorBase>>, Validators)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TArray<TWeakObjectPtr<UBlueprintValidatorBase>> LocalValidators;
	
	TSharedPtr<SListView<TWeakObjectPtr<UBlueprintValidatorBase>>> ListViewWidget;
	
	TSharedPtr<FSlateStyleSet> CheckBoxStyleSet;

	FSlateFontInfo FontInfo;

	TSharedRef<ITableRow> OnGenerateRowForList(TWeakObjectPtr<UBlueprintValidatorBase> InItem, const TSharedRef<STableViewBase>& OwnerTable);
};
