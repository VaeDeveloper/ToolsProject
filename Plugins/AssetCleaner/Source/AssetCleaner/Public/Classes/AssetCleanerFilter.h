// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace AssetCleaner
{
	class IAssetFilter
	{
	public:
		virtual ~IAssetFilter() {};
		virtual bool PassesFilter(const FAssetData& AssetData) const = 0;
		virtual FString GetFilterName() const = 0;
		virtual FString GetFilterTooltip() const = 0;
	};


	class FLambdaAssetFilter : public IAssetFilter
	{
	public:
		FLambdaAssetFilter(const FString& InName, const FString& InTooltip, TFunctionRef<bool(const FAssetData&)> InPredicate)
			: Name(InName), Tooltip(InTooltip), Predicate(InPredicate)
		{
		}

		virtual FString GetFilterName() const override { return Name; }
		virtual FString GetFilterTooltip() const override { return Tooltip; }
		virtual bool PassesFilter(const FAssetData& Asset) const override { return Predicate(Asset); }

	private:
		FString Name;
		FString Tooltip;
		TFunctionRef<bool(const FAssetData&)> Predicate;
	};
}