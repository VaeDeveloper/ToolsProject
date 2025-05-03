// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseClasses/BlueprintValidatorBase.h"

/**
 * 
 */
class VALIDATORX_API FValidatorXManager
{
	FValidatorXManager() {}
	FValidatorXManager(const FValidatorXManager&) = delete;
	FValidatorXManager& operator=(const FValidatorXManager&) = delete;

public:
	static FValidatorXManager& Get()
	{
		static FValidatorXManager Istance;
		return Istance;
	}

	void RegisterValidator(UBlueprintValidatorBase* Validator)
	{
		if (Validator)
		{
			Validators.Add(Validator);
		}
	}

	const TArray<TWeakObjectPtr<UBlueprintValidatorBase>>& GetValidators() const
	{
		return Validators;
	}

private:
	TArray<TWeakObjectPtr<UBlueprintValidatorBase>> Validators;

};
