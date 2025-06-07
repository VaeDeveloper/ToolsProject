// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Texture.h"
#include "Blueprint/UserWidget.h"
#include "Animation/MorphTarget.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/Level.h"
#include "GameFramework/GameModeBase.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AimOffsetBlendSpace1D.h"
#include "Animation/BlendSpace.h"
//#include "Animation/BlendSpaceBase.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/PoseAsset.h"
#include "Animation/AnimComposite.h"
#include "Animation/AnimLayerInterface.h"
//#include "ContextualAnimSceneAsset.h"
#include "Animation/MirrorDataTable.h"
//#include "AnimationSharingSetup.h"
//#include "NiagaraSystem.h"
//#include "NiagaraEmitter.h"


struct FAssetNamingConfig
{
	/**
	 * A map that associates asset classes with their corresponding naming prefixes.
	 *
	 * This map is used to apply consistent prefixes to asset names when they are renamed or organized.
	 */

	 /*..........................................................................................................*/
	const TMap<UClass*, FString> PrefixMap =
	{
		{UBlueprint::StaticClass(), TEXT("BP_")},
		{UTexture::StaticClass(), TEXT("T_")},
		{UStaticMesh::StaticClass(), TEXT("SM_")},
		{USoundCue::StaticClass(), TEXT("SC_")},
		{UMaterial::StaticClass(), TEXT("M_")},
		{USoundWave::StaticClass(), TEXT("S_")},
		{UTexture2D::StaticClass(), TEXT("T_")},
		{UUserWidget::StaticClass(), TEXT("UW_")},
		{UMorphTarget::StaticClass(), TEXT("MT_")},
		{UParticleSystem::StaticClass(), TEXT("PS_")},
		{UMaterialInstance::StaticClass(), TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
		{USkeletalMeshComponent::StaticClass(), TEXT("SK_")},
		{UAnimSequence::StaticClass(), TEXT("Anim_")},
		{UAnimMontage::StaticClass(), TEXT("AnimM_")},
		{UAnimBlueprint::StaticClass(), TEXT("ABP_")},
		{ULevel::StaticClass(), TEXT("Map_")},
		{AGameModeBase::StaticClass(), TEXT("GM_")},
		{UBlendSpace::StaticClass(), TEXT("BS_")},
		{UBlendSpace1D::StaticClass(), TEXT("BS1D_")},
		{UPoseAsset::StaticClass(), TEXT("PA_")},
		{UAimOffsetBlendSpace::StaticClass(), TEXT("AO_")},
		{UAimOffsetBlendSpace1D::StaticClass(), TEXT("AO1D_")},
		{UAnimComposite::StaticClass(), TEXT("AC_")},
		{UAnimLayerInterface::StaticClass(), TEXT("ALI_")},
		{UMirrorDataTable::StaticClass(), TEXT("MirDT_")}
	};

	/*..........................................................................................................*/
};



class FContentBrowserToolkitModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;



private:

	void InitContentBrowserMenuExtension();
	TSharedRef<FExtender> CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths);
	void AddContentBrowserMenuEntry(FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetClicked();
	void FindDuplicateAssets();


	void OnDeleteEmptyFoldersClicked();

	void PopulateAssetActionSubmenu(FMenuBuilder& MenuBuilder);
	void ShowDuplicateAssetsWindow(const TArray<TSharedPtr<struct FDuplicateAssetInfo>>& DuplicateAssets);

	void AddPrefix();

	TArray<FString> FolderPathsSelected;

	FAssetNamingConfig AssetRenameConfig;
};
