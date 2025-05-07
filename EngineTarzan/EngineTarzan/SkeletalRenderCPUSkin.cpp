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

    FSkeletalMeshRenderData* SkeletalMeshRenderData = SkeletalMesh->GetRenderData();
    TArray<FSoftSkinVertex> Vertices = InMeshComponent->GetBindPoseVertices();
    TArray<FSkeletalMeshVertex>& RenderDataVertices = SkeletalMeshRenderData->Vertices;
    int VertexIndex = 0;
    for (const auto& Vertex : Vertices)
    {
        SkinVertex(Vertex, InverseBindPose, GlobalTransforms, RenderDataVertices[VertexIndex]);
        VertexIndex += 1;
    }
}

void FSkeletalMeshObjectCPUSkin::SkinVertex(const FSoftSkinVertex& Vertex, TArray<FMatrix> InverseBindPose, TArray<FTransform> BoneGlobalTransforms, FSkeletalMeshVertex& OutVertex)
{
    FVector SkinnedPos = FVector::ZeroVector;
    FVector4 SkinnedNormal = FVector4(0,0,0,0);
    FVector SkinnedTangent = FVector::ZeroVector;

    const FVector& OriginalPos = Vertex.Position;
    const FVector4& OriginalNormal = Vertex.TangentZ;
    const FVector& OriginalTangent = Vertex.TangentX;

    bool bHasInfluence = false;

    for (int i = 0; i < MAX_TOTAL_INFLUENCES; ++i)
    {
        const uint8& BoneIndex = Vertex.InfluenceBones[i];
        const float& Weight = Vertex.InfluenceWeights[i];
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
        OutVertex.X = SkinnedPos.X;
        OutVertex.Y = SkinnedPos.Y;
        OutVertex.Z = SkinnedPos.Z;
        FVector Normal = FVector(SkinnedNormal.X, SkinnedNormal.Y, SkinnedNormal.Z).GetSafeNormal();
        OutVertex.NormalX = Normal.X;
        OutVertex.NormalY = Normal.Y;
        OutVertex.NormalZ = Normal.Z;
        FVector Tangent = SkinnedTangent.GetSafeNormal();
        OutVertex.TangentX = Tangent.X;
        OutVertex.TangentY = Tangent.Y;
        OutVertex.TangentZ = Tangent.Z;
    }
}
