// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectFilter/ObjectMixerEditorObjectFilter.h"
#include "RayTracingObjectFilter.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew)
class URayTracingObjectFilter : public UObjectMixerObjectFilter
{
	GENERATED_BODY()

public:
	virtual TSet<UClass*> GetObjectClassesToFilter() const override
	{
		return
		{
			UPrimitiveComponent::StaticClass()
		};
	}

	virtual TSet<TSubclassOf<AActor>> GetObjectClassesToPlace() const override
	{
		return
		{
			AActor::StaticClass()
		};
	}

	virtual TSet<FName> GetColumnsToShowByDefault() const override
	{
		return
		{
			"RayTracingGroupID", "RayTracingGroupCullingPriority"
		};
	}

	virtual TSet<FName> GetForceAddedColumns() const override
	{
		return
		{
			{"RayTracingGroupID", "RayTracingGroupCullingPriority"}
		};
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
