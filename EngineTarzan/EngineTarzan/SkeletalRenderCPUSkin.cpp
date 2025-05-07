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

    TArray<FMatrix> InverseBindPose = Skeleton.GetInverseBindPose();
    TArray<FTransform> GlobalTransforms = InMeshComponent->GetWorldSpaceTransforms();

    FSkeletalMeshLODModel* SkeletalMeshData = SkeletalMesh->GetImportedModel();
    TArray<FSoftSkinVertex> Vertices = InMeshComponent->GetBindPoseVertices();
    TArray<FSoftSkinVertex>& LODVertices = SkeletalMeshData->Vertices;
    int vertex = 0;
    for (auto& Vertex : Vertices)
    {
        SkinVertex(Vertex, InverseBindPose, GlobalTransforms);
        LODVertices[vertex] = Vertex;
        vertex += 1;
    }
    FSkeletalMeshBuilder::ConvertLODModelToRenderData(*SkeletalMeshData, *SkeletalMesh->GetRenderData());
}

void FSkeletalMeshObjectCPUSkin::SkinVertex(FSoftSkinVertex& Vertex, TArray<FMatrix> InverseBindPose, TArray<FTransform> BoneGlobalTransforms)
{
    FVector SkinnedPos = FVector::ZeroVector;
    FVector4 SkinnedNormal = FVector4(0,0,0,0);
    FVector SkinnedTangent = FVector::ZeroVector;
    FVector SkinnedBiTangent = FVector::ZeroVector;

    FVector& OriginalPos = Vertex.Position;
    FVector4& OriginalNormal = Vertex.TangentZ;
    FVector& OriginalTangent = Vertex.TangentX;

    bool bHasInfluence = false;

    for (int i = 0; i < MAX_TOTAL_INFLUENCES; ++i)
    {
        uint8& BoneIndex = Vertex.InfluenceBones[i];
        float& Weight = Vertex.InfluenceWeights[i];
        if (Weight <= KINDA_SMALL_NUMBER)
        {
            continue;
        }

        bHasInfluence = true;

        const FMatrix BoneMatrix = BoneGlobalTransforms[BoneIndex].GetMatrix();
        FMatrix SkinningMatrix = InverseBindPose[BoneIndex] * BoneMatrix;

        SkinnedPos += SkinningMatrix.TransformPosition(OriginalPos) * Weight;
        SkinnedNormal += SkinningMatrix.TransformFVector4(OriginalNormal) * Weight;
        SkinnedTangent += FMatrix::TransformVector(OriginalTangent, SkinningMatrix) * Weight;
    }
    
    if (bHasInfluence)
    {
        Vertex.Position = SkinnedPos;
        FVector Normal = FVector(SkinnedNormal.X, SkinnedNormal.Y, SkinnedNormal.Z).GetSafeNormal();
        Vertex.TangentZ = FVector4(Normal, Vertex.TangentZ.W);
        Vertex.TangentX = SkinnedTangent.GetSafeNormal();
        Vertex.TangentY = (FVector::CrossProduct(Normal, Vertex.TangentX)) * Vertex.TangentZ.W;
    }
}
