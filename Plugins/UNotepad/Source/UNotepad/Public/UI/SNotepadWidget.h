// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
/**
 * 
 */
class UNOTEPAD_API SNotepadWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNotepadWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

};
