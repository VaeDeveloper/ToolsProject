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
	TObjectPtr<UAnimMontage> TestAnimMontage;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> TestAnimMontage;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> TestAnimMontage;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> TestAnimMontage;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCurveFloat> TestCurveClass
	
};
