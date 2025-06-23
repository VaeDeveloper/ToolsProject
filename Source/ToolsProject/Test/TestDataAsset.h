// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TestDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class TOOLSPROJECT_API UTestDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> TestAnimMontage1;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> TestAnimMontage2;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> TestAnimMontage3;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> TestAnimMontage4;
	
};
