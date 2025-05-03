// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "Interface/ValidatorToggleInterface.h"
#include "BlueprintValidatorBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class VALIDATORX_API UBlueprintValidatorBase : public UEditorValidatorBase, public IValidatorToggleInterface
{
	GENERATED_BODY()

public:
	UBlueprintValidatorBase()
	{
		bIsEnabled = true;
	}

	virtual FString GetTypeValidator() const
	{
		return TEXT("Blueprint");
	}
#pragma region IValidatorToggleInterface
	virtual void ToggleValidationEnabled() override {}
	virtual void SetValidationEnabled(bool bEnabled) override {}
#pragma endregion
	bool bIsError = false;

};
