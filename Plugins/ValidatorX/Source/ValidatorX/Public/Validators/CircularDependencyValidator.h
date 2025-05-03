// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseClasses/BlueprintValidatorBase.h"
#include "CircularDependencyValidator.generated.h"

/**
 * 
 */
UCLASS()
class VALIDATORX_API UCircularDependencyValidator : public UBlueprintValidatorBase
{
	GENERATED_BODY()

public:
	UCircularDependencyValidator();

	virtual void SetValidationEnabled(bool bEnabled) override;


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


private:
	bool HasCircularDependency(UBlueprint* Blueprint, FDataValidationContext& Context);
	bool DetectCycle(const FName& StartName, TMap<FName, TArray<FName>>& GraphMap, TSet<FName>& Visited, TSet<FName>& Stack, TArray<FName>& OutCyclePath);
	UEdGraph* FindGraphByName(UBlueprint* Blueprint, const FName& GraphName);

};
