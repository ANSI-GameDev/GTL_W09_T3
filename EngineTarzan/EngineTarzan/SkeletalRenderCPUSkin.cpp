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

    // 1) 역바인드포즈 × 본글로벌행렬 합성 → SkinnedMatrices 에 저장
    const int32 NumBones = InverseBindPose.Num();
    TArray<FMatrix> SkinnedMatrices;
    SkinnedMatrices.SetNum(NumBones);
    for (int32 b = 0; b < NumBones; ++b)
    {
        FMatrix BoneM = GlobalTransforms[b].GetMatrix();
        SkinnedMatrices[b] = InverseBindPose[b] * BoneM;
    }

    const TArray<FSoftSkinVertex>& Vertices = InMeshComponent->GetBindPoseVertices();
    TArray<FSkeletalMeshVertex>& RenderDataVertices = SkeletalMesh->GetRenderData()->Vertices;
    //int VertexIndex = 0;
    //for (const auto& Vertex : Vertices)
    //{
    //    SkinVertex(Vertex, InverseBindPose, GlobalTransforms, RenderDataVertices[VertexIndex]);
    //    VertexIndex += 1;
    //}

    const int32 NumVerts = Vertices.Num();
    for (int32 i = 0; i < NumVerts; ++i)
    {
        SkinVertexOptimized(Vertices[i],
            SkinnedMatrices,
            RenderDataVertices[i]);
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


void FSkeletalMeshObjectCPUSkin::SkinVertexOptimized(
    const FSoftSkinVertex& Src,
    const TArray<FMatrix>& SkinnedMatrices,
    FSkeletalMeshVertex& Out)
{
    // 누적 변수
    FVector   Psum(0, 0, 0);
    FVector4  Nsum(0, 0, 0, 0);
    FVector   Tsum(0, 0, 0);

    // 각 인플루언스마다 미리 계산된 합성행렬만 사용
    for (int i = 0; i < MAX_TOTAL_INFLUENCES; ++i)
    {
        const float W = Src.InfluenceWeights[i];
        if (W <= KINDA_SMALL_NUMBER) continue;

        const int   Bi = Src.InfluenceBones[i];
        const FMatrix& M = SkinnedMatrices[Bi];

        Psum += M.TransformPosition(Src.Position) * W;
        Nsum += M.TransformFVector4(Src.TangentZ) * W;
        Tsum += FMatrix::TransformVector(Src.TangentX, M) * W;
    }

    // 결과 정규화 후 아웃풋에 복사
    Out.X = Psum.X;  Out.Y = Psum.Y;  Out.Z = Psum.Z;

    FVector N = FVector(Nsum.X, Nsum.Y, Nsum.Z).GetSafeNormal();
    Out.NormalX = N.X;  Out.NormalY = N.Y;  Out.NormalZ = N.Z;

    FVector T = Tsum.GetSafeNormal();
    Out.TangentX = T.X;  Out.TangentY = T.Y;  Out.TangentZ = T.Z;
}
