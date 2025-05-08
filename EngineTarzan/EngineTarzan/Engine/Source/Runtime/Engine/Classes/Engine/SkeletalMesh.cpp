#include "SkeletalMesh.h"

#include "Asset/SkeletalMeshAsset.h"

USkeletalMeshAsset::USkeletalMeshAsset()
{
}

USkeletalMeshAsset::~USkeletalMeshAsset()
{
}

UObject* USkeletalMeshAsset::Duplicate(UObject* InOuter)
{
    return Super::Duplicate(InOuter);
}

bool USkeletalMeshAsset::InitializeFromData(const std::shared_ptr<FAssetData> Data)
{
    // 타입 확인 후 다운캐스트
    const std::shared_ptr<FSkeletalAssetData> SkeletalData = std::dynamic_pointer_cast<FSkeletalAssetData>(Data);
    if (!SkeletalData)
    {
        UE_LOG(LogLevel::Warning, TEXT("InitializeFromData failed: Data is not FSkeletalAssetData."));
        return false;
    }

    // 중간 데이터 복사
    AssetData = SkeletalData;

    // TODO : RenderData 생성 및 초기화
    SkelMeshRenderData = std::make_shared<FSkeletalMeshRenderData>();

    UE_LOG(LogLevel::Display, TEXT("USkeletalMeshAsset initialized: %s"), *GetMetaData().AssetName.ToString());
    return true;
}

void USkeletalMeshAsset::Unload()
{
}

EAssetType USkeletalMeshAsset::GetAssetType() const
{
    return EAssetType::SkeletalMesh;
}

void USkeletalMeshAsset::Initialize()
{
}

void USkeletalMeshAsset::SetImportedModel(const std::shared_ptr<FSkeletalMeshLODModel>& InModel)
{
    ImportedModel = InModel;
}

void USkeletalMesh::SetSkeletalMeshRenderData(FSkeletalMeshRenderData* InRenderData)
{
    SkelMeshRenderData = InRenderData;
}

void USkeletalMesh::SetRefSkeleton(const FReferenceSkeleton& InRefSkeleton)
{
    RefSkeleton = InRefSkeleton;
}

FSkeletalMeshRenderData* USkeletalMesh::GetRenderData() const
//std::shared_ptr<FSkeletalMeshRenderData> USkeletalMeshAsset::GetSkeletalMeshRenderData() const
{
    return SkelMeshRenderData;
}

FSkeletalMeshLODModel* USkeletalMesh::GetImportedModel() const
{
    return ImportedModel;
}

const FReferenceSkeleton& USkeletalMesh::GetRefSkeleton() const
{
    return RefSkeleton;
}

