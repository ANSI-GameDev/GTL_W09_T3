#include "SkeletalRenderCPUSkin.h"

#include "Rendering/SkeletalMeshLODModel.h"

#include "Components/SkeletalMeshComponent.h"

#include "Engine/Asset/SkeletalMeshAsset.h"

#include "ReferenceSkeleton.h"

#include "Engine/SkeletalMesh.h"

#include "Developer/SkeletalMeshBuilder.h"


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
    TArray<FTransform> GlobalTransforms = InMeshComponent->GetWorldSpaceTransforms();

    FSkeletalMeshLODModel* SkeletalMeshData = SkeletalMesh->GetImportedModel();
    TArray<FSoftSkinVertex> Vertices = SkeletalMeshData->Vertices;
    for (auto& Vertex : SkeletalMeshData->Vertices)
    {
        SkinVertex(Vertex, BindPoseTransforms, GlobalTransforms);
    }
    FSkeletalMeshBuilder::ConvertLODModelToRenderData(*SkeletalMeshData, *SkeletalMesh->GetRenderData());
}

void FSkeletalMeshObjectCPUSkin::SkinVertex(FSoftSkinVertex& Vertex, TArray<FTransform> BindPoseTransforms, TArray<FTransform> GlobalTransforms)
{
    FVector ResultPosition = { 0,0,0 };
    FVector4 ResultNormal = { 0,0,0,0 };
    FVector ResultTangent = { 0,0,0 };
    FVector ResultBiTangent = { 0,0,0 };

    FVector OriginalPos = Vertex.Position;
    FVector4 OriginalNormal = Vertex.TangentZ;
    FVector OriginalTangent = Vertex.TangentX;

    FMatrix SkeletonPose;
    FMatrix InverseBindPose;
    FMatrix SkinMatrix;

    bool bHasBone = false;

    for (int i = 0; i < MAX_TOTAL_INFLUENCES; ++i)
    {
        if (Vertex.InfluenceWeights[i] <= 0.0f)
            continue;
        bHasBone = true;
        SkeletonPose = GlobalTransforms[Vertex.InfluenceBones[i]].GetMatrix();
        InverseBindPose = FMatrix::Inverse(BindPoseTransforms[Vertex.InfluenceBones[i]].GetMatrix());
        SkinMatrix = SkeletonPose * InverseBindPose;
        ResultPosition += FMatrix::TransformVector(OriginalPos, SkinMatrix) * Vertex.InfluenceWeights[i];
        //ResultPosition += SkinMatrix.TransformPosition(OriginalPos) * Vertex.InfluenceWeights[i];
        ResultNormal += SkinMatrix.TransformFVector4(OriginalNormal) * Vertex.InfluenceWeights[i];
        ResultTangent += FMatrix::TransformVector(OriginalTangent, SkinMatrix) * Vertex.InfluenceWeights[i];
    }
    
    if (bHasBone)
    {
        Vertex.Position = ResultPosition;  
        FVector Normal = FVector(ResultNormal.X, ResultNormal.Y, ResultNormal.Z).GetSafeNormal();
        Vertex.TangentZ = FVector4(Normal, Vertex.TangentZ.W);
        Vertex.TangentX = ResultTangent.GetSafeNormal();
        Vertex.TangentY = (FVector::CrossProduct(Normal, Vertex.TangentX)) * Vertex.TangentZ.W;
    }
}
