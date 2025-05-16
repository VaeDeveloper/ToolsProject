// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectFilter/ObjectMixerEditorObjectFilter.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicsObjectFilter.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALOBJECTMIXER_API UPhysicsObjectFilter : public UObjectMixerObjectFilter
{
	GENERATED_BODY()

public:

	virtual TSet<UClass*> GetObjectClassesToFilter() const override
	{
		return
		{
			UStaticMeshComponent::StaticClass()
		};
	}

	virtual TSet<TSubclassOf<AActor>> GetObjectClassesToPlace() const override
	{
		return
		{
			AStaticMeshActor::StaticClass()
		};
	}

	virtual TSet<FName> GetColumnsToShowByDefault() const override
	{
		return
		{
			"SimulatePhysics",
			"MassInKg",
			"LinearDamping",
			"AngularDamping",
			"Mobility",
			"CollisionEnabled",
			"CollisionProfileName",
			"bGenerateOverlapEvents",
			"bUseCCD"
		};
	}

	virtual TSet<FName> GetForceAddedColumns() const override
	{
		return {};
	}

	virtual bool GetShowTransientObjects() const override
	{
		return false;
	}

	virtual bool ShouldIncludeUnsupportedProperties() const override
	{
		return false;
	}

	virtual EObjectMixerInheritanceInclusionOptions GetObjectMixerPropertyInheritanceInclusionOptions() const override
	{
		return EObjectMixerInheritanceInclusionOptions::IncludeAllParentsAndChildren;
	}

	virtual EObjectMixerInheritanceInclusionOptions GetObjectMixerPlacementClassInclusionOptions() const override
	{
		return EObjectMixerInheritanceInclusionOptions::IncludeAllChildren;
	}
	
};
