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

FSkeletalMeshRenderData* USkeletalMesh::GetSkeletalMeshRenderData() const
{
    return SkelMeshRenderData;
}

