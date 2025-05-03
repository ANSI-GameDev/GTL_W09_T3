#include "SkeletalMesh.h"

#include "Asset/SkeletalMeshAsset.h"

void USkeletalMesh::SetImportedModel(FSkeletalMeshLODModel* InModel)
{
    ImportedModel = InModel;
}

FSkeletalMeshRenderData* USkeletalMesh::GetSkeletalMeshRenderData() const
{
    return SkelMeshRenderData;
}

