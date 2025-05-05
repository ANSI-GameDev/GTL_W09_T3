#include "SkeletalMesh.h"

#include "Asset/SkeletalMeshAsset.h"

USkeletalMesh::USkeletalMesh()
    : ImportedModel(nullptr)
    , SkelMeshRenderData(nullptr)
{
    // Initialize the ImportedModel and SkelMeshRenderData
    
}

void USkeletalMesh::Initialize()
{
    ImportedModel = new FSkeletalMeshLODModel();
    SkelMeshRenderData = new FSkeletalMeshRenderData();
}

void USkeletalMesh::SetImportedModel(FSkeletalMeshLODModel* InModel)
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

FSkeletalMeshRenderData* USkeletalMesh::GetSkeletalMeshRenderData() const
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

