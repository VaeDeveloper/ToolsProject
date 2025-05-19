// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/AssetFilterLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"

TSet<FName> AssetCleaner::FAssetFilterLibrary::AssetsWithMetadata{};
TSet<FName> AssetCleaner::FAssetFilterLibrary::TexturesWithoutCompression{};
TSet<FName> AssetCleaner::FAssetFilterLibrary::AssetsWithInvalidReferences{};
TSet<FName> AssetCleaner::FAssetFilterLibrary::TexturesWithWrongSize{};
TSet<FName> AssetCleaner::FAssetFilterLibrary::FilteredMaterials{};

bool AssetCleaner::FAssetFilterLibrary::IsAssetUnreferenced(const FAssetData& Asset)
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FName> Referencers;
	AssetRegistry.GetReferencers(Asset.PackageName, Referencers);

	return Referencers.Num() == 0;
}

bool AssetCleaner::FAssetFilterLibrary::IsAssetWithMissingReferences(const FAssetData& Asset)
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FAssetDependency> Dependencies;
	const FAssetIdentifier AssetId(Asset.PackageName);

	const bool bGotDeps = AssetRegistry.GetDependencies(
		AssetId,
		Dependencies,
		UE::AssetRegistry::EDependencyCategory::Package,
		UE::AssetRegistry::FDependencyQuery(UE::AssetRegistry::EDependencyQuery::Hard)
	);

	if(!bGotDeps)
	{
		return false;
	}

	for(const FAssetDependency& Dependency : Dependencies)
	{
		TArray<FAssetData> DependentAssets;
		AssetRegistry.GetAssetsByPackageName(Dependency.AssetId.PackageName, DependentAssets);
		if(DependentAssets.Num() == 0)
		{
			return true;
		}
	}

	return false;
}

void AssetCleaner::FAssetFilterLibrary::CollectMetadata(const TArray<TSharedPtr<FAssetData>>& InAssetList)
{
	AssetsWithMetadata.Empty();

	FScopedSlowTask SlowTask(InAssetList.Num(), FText::FromString(TEXT("Collecting assets with metadata...")));
	SlowTask.MakeDialog(true);

	for(const TSharedPtr<FAssetData>& Asset : InAssetList)
	{
		SlowTask.EnterProgressFrame(1);

		if(!Asset.IsValid()) continue;

		FSoftObjectPath SoftPath = Asset->ToSoftObjectPath();
		UObject* LoadedObject = SoftPath.ResolveObject();

		if(!LoadedObject)
		{
			LoadedObject = SoftPath.TryLoad();
		}

		if(!LoadedObject || LoadedObject->HasAnyFlags(RF_NeedLoad | RF_NeedPostLoad))
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping %s — not fully loaded."), *Asset->GetObjectPathString());
			continue;
		}

		if(const TMap<FName, FString>* MetaMap = UMetaData::GetMapForObject(LoadedObject))
		{
			if(MetaMap->Num() > 0)
			{
				AssetsWithMetadata.Add(Asset->PackageName);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Found %d assets with metadata."), AssetsWithMetadata.Num());
}

void AssetCleaner::FAssetFilterLibrary::CollectAssetsWithInvalidReferences(const TArray<TSharedPtr<FAssetData>>& InAssetList)
{
	AssetsWithInvalidReferences.Empty();

	FScopedSlowTask SlowTask(InAssetList.Num(), FText::FromString(TEXT("Checking assets for invalid references...")));
	SlowTask.MakeDialog(true);

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	for(const TSharedPtr<FAssetData>& Asset : InAssetList)
	{
		SlowTask.EnterProgressFrame(1.f);

		if(!Asset.IsValid()) continue;

		TArray<FAssetIdentifier> References;
		AssetRegistry.GetReferencers(Asset->GetPrimaryAssetId(),
			References, UE::AssetRegistry::EDependencyCategory::All);

		for(const FAssetIdentifier& Ref : References)
		{
			// ignore self-reference
			if(Ref == Asset->GetPrimaryAssetId()) continue;

			FString PackageName = Ref.PackageName.ToString();
			if(!FPackageName::DoesPackageExist(PackageName))
			{
				AssetsWithInvalidReferences.Add(Asset->PackageName);
				break;
			}
		}
	}

}

void AssetCleaner::FAssetFilterLibrary::CollectTexturesWithoutCompression(const TArray<TSharedPtr<FAssetData>>& InAssetList)
{
	TexturesWithoutCompression.Empty();

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	FTopLevelAssetPath TextureClassPath(TEXT("/Script/Engine.Texture2D"));

	TArray<FAssetData> AllTextures;
	AssetRegistry.GetAssetsByClass(TextureClassPath, AllTextures);

	FScopedSlowTask SlowTask(AllTextures.Num(), FText::FromString(TEXT("Scanning Textures Without Compression...")));
	SlowTask.MakeDialog(true);

	for(const FAssetData& TextureAsset : AllTextures)
	{
		SlowTask.EnterProgressFrame(1.f, FText::FromString(TextureAsset.AssetName.ToString()));

		if(UTexture2D* Texture2D = Cast<UTexture2D>(TextureAsset.GetAsset()))
		{
			if(Texture2D->CompressionSettings == TextureCompressionSettings::TC_VectorDisplacementmap ||
				Texture2D->CompressionSettings == TextureCompressionSettings::TC_Grayscale)
			{
				TexturesWithoutCompression.Add(TextureAsset.PackageName);
			}
		}
	}

}

void AssetCleaner::FAssetFilterLibrary::CollectTexturesWithWrongSize(const TArray<TSharedPtr<FAssetData>>& InAssetList)
{
	TexturesWithWrongSize.Empty();

	TArray<TSharedPtr<FAssetData>> TextureAssets;
	for(const TSharedPtr<FAssetData>& Asset : InAssetList)
	{
		if(Asset.IsValid() && Asset->AssetClassPath == UTexture2D::StaticClass()->GetClassPathName())
		{
			TextureAssets.Add(Asset);
		}
	}

	FScopedSlowTask SlowTask(TextureAssets.Num(), FText::FromString(TEXT("Scanning textures with wrong size (Non-Po2)...")));
	SlowTask.MakeDialog(true);

	auto IsPowerOfTwo = [] (int32 Value) -> bool {
		return Value > 0 && (Value & (Value - 1)) == 0;
		};

	for(const TSharedPtr<FAssetData>& Asset : TextureAssets)
	{
		SlowTask.EnterProgressFrame(1);

		UObject* LoadedObject = Asset->GetAsset();
		if(!LoadedObject) continue;

		if(UTexture2D* Texture = Cast<UTexture2D>(LoadedObject))
		{
			int32 Width = Texture->GetSizeX();
			int32 Height = Texture->GetSizeY();

			if(!IsPowerOfTwo(Width) || !IsPowerOfTwo(Height))
			{
				TexturesWithWrongSize.Add(Asset->PackageName);
			}
		}
	}

}
