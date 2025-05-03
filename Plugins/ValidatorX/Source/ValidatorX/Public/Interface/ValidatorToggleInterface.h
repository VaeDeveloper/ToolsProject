// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ValidatorToggleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UValidatorToggleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class VALIDATORX_API IValidatorToggleInterface
{
	GENERATED_BODY()

public:
	/**
	 * Toggles the enabled state of this validator
	 */
	virtual void ToggleValidationEnabled() = 0;

	/**
	 * Sets the enabled state of this validator
	 *
	 * @param bEnabled  True to enable validation, false to disable
	 */
	virtual void SetValidationEnabled(bool bEnabled) = 0;
};
