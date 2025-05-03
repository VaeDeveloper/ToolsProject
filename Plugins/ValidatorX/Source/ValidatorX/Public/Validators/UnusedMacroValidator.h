// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseClasses/BlueprintValidatorBase.h"
#include "UnusedMacroValidator.generated.h"

/**
 * 
 */
UCLASS()
class VALIDATORX_API UUnusedMacroValidator : public UBlueprintValidatorBase
{
	GENERATED_BODY()

public:
	UUnusedMacroValidator();

	virtual void SetValidationEnabled(bool bEnabled) override
	{
		static UUnusedMacroValidator* CDO = GetMutableDefault<UUnusedMacroValidator>();
		if(bIsConfigDisabled)
		{
			UE_LOG(LogTemp, Warning, TEXT("Validator is disabled by config!"));
			return;
		}

		CDO->bIsEnabled = bEnabled;
		SaveConfig();
	}

	/**
	 * Checks if the validator is currently enabled.
	 *
	 * @return True if validation is active
	 */
	virtual bool IsEnabled() const override;

	/**
	 * Checks whether this validator can validate the given asset.
	 *
	 * @param InAssetData   Asset metadata (path, type, etc.)
	 * @param InObject      Loaded asset object (null if not loaded)
	 * @param InContext     Validation context for error/warning accumulation
	 * @return True if this validator should process the asset
	 */
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const override;

	/**
	 * Performs validation on a loaded asset.
	 *
	 * @param InAssetData   Asset metadata
	 * @param InAsset       Loaded asset object
	 * @param Context       Validation context for reporting issues
	 * @return EDataValidationResult::Passed if valid, Failed/Invalid otherwise
	 */
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
	
};
