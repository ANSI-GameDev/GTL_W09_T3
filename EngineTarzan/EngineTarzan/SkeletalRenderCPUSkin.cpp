#include "SkeletalRenderCPUSkin.h"

#include "Container/Array.h"

#include "Rendering/SkeletalMeshLODModel.h"

#include "Components/SkeletalMeshComponent.h"

#include "Engine/Asset/SkeletalMeshAsset.h"

#include "ReferenceSkeleton.h"

#include "Engine/SkeletalMesh.h"


void FSkeletalMeshObjectCPUSkin::InitResources(USkinnedMeshComponent* InMeshComponent, FSkeletalMeshRenderData* InSkelMeshRenderData)
{
    MeshComponent = InMeshComponent;
    SkelMeshRenderData = InSkelMeshRenderData;
}

void FSkeletalMeshObjectCPUSkin::Update(USkinnedMeshComponent* InMeshComponent, float DeltaTime)
{
    // @TODO : CPU Skinning 로직을 여기에 구현
    // 1. SkeletalMeshRenderData에서 Vertices, Indices 가져오기
    // 2. ComponentSpaceTransformsArray에서 Bone Transform 가져오기
    // 3. Vertex를 Bone Transform에 맞게 변환
    // 4. 변환된 Vertex를 Weight를 적절히 적용하여 Skinning을 구현할 것

    //FSkeletalMeshLODModel*
    USkeletalMesh* SkeletalMesh = InMeshComponent->GetSkeletalMesh();
    FReferenceSkeleton& Skeleton = SkeletalMesh->RefSkeleton;

    TArray<FTransform> BindPoseTransforms = Skeleton.GetBonePose();
    TArray<FTransform> GlobalTransforms = InMeshComponent->GetComponentSpaceTransforms();

    FSkeletalMeshLODModel* SkeletalMeshData = SkeletalMesh->GetImportedModel();

    for (auto& Vertex : SkeletalMeshData->Vertices)
    {
        FVector OriginalPos = Vertex.Position;
        FVector Result = { 0,0,0 };
        
        FMatrix SkeletonPose;
        FMatrix InverseBindPose;
        FMatrix SkinMatrix;
        for (int i=0; i< MAX_TOTAL_INFLUENCES; ++i)
        {
            if (Vertex.InfluenceWeights[i] <= 0.0f)
                continue;
            SkeletonPose = GlobalTransforms[Vertex.InfluenceBones[i]].GetMatrix();
            InverseBindPose = FMatrix::Inverse(BindPoseTransforms[Vertex.InfluenceBones[i]].GetMatrix());
            SkinMatrix = SkeletonPose * InverseBindPose;
            Result += FMatrix::TransformVector(OriginalPos, SkinMatrix) * Vertex.InfluenceWeights[i];
        }
        Vertex.Position = Result;
    }
}
