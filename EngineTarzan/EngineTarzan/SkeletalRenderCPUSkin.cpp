#include "SkeletalRenderCPUSkin.h"

#include "Rendering/SkeletalMeshLODModel.h"

#include "Components/SkeletalMeshComponent.h"

#include "Engine/Asset/SkeletalMeshAsset.h"

#include "ReferenceSkeleton.h"

#include "Engine/SkeletalMesh.h"

#include "Developer/SkeletalMeshBuilder.h"
#include "Math/DualQuat.h"

//#define DUAL_QUATERNION 

void FSkeletalMeshObjectCPUSkin::InitResources(USkinnedMeshComponent* InMeshComponent, FSkeletalMeshRenderData* InSkelMeshRenderData)
{
    MeshComponent = InMeshComponent;
    SkelMeshRenderData = InSkelMeshRenderData;
}

#ifdef DUAL_QUATERNION 
void FSkeletalMeshObjectCPUSkin::Update(USkinnedMeshComponent* InMeshComponent)
{
    USkeletalMesh* SkeletalMesh = InMeshComponent->GetSkeletalMesh();
    FReferenceSkeleton& Skeleton = SkeletalMesh->RefSkeleton;

    TArray<FMatrix> InverseBindPose = Skeleton.GetInverseBindPose();
    TArray<FTransform> GlobalTransforms = InMeshComponent->GetWorldSpaceTransforms();

    const int32 NumBones = InverseBindPose.Num();
    TArray<FDualQuat> SkinDQs;
    SkinDQs.SetNum(NumBones);

    // 본별로 듀얼 쿼터니언 생성 (InvBind * Global)
    for (int32 b = 0; b < NumBones; ++b)
    {
        FMatrix M = InverseBindPose[b] * GlobalTransforms[b].GetMatrix();
        // M에서 회전+병렬벡터 분해
        FQuat    Rot(M);               // FMatrix → 회전 쿼터니언
        FVector  Trans = M.GetOrigin(); // M의 이동 성분
        SkinDQs[b] = FDualQuat(Rot, Trans).GetNormalized();
    }

    const TArray<FSoftSkinVertex>& SrcVerts = InMeshComponent->GetBindPoseVertices();
    TArray<FSkeletalMeshVertex>& DstVerts = SkeletalMesh->GetRenderData()->Vertices;

    const int32 NumVerts = SrcVerts.Num();
    for (int32 i = 0; i < NumVerts; ++i)
    {
        SkinVertexDualQuat(SrcVerts[i], SkinDQs, DstVerts[i]);
    }
}
void FSkeletalMeshObjectCPUSkin::SkinVertexDualQuat(
    const FSoftSkinVertex& Src,
    const TArray<FDualQuat>& SkinDQs,
    FSkeletalMeshVertex& Out)
{
    // 가중치 합산용 듀얼 쿼터니언 (초기화)
    FDualQuat BlendedDQ; // 기본 생성자: Real=(0,0,0,1), Dual=(0,0,0,0) -> 합산 시작시 문제될 수 있음
    // 명시적으로 0으로 초기화 하거나 첫번째 영향으로 초기화 하는 것이 좋음
    BlendedDQ.Real = FQuat(0.f, 0.f, 0.f, 0.f); // 합산을 위해 0으로 초기화
    BlendedDQ.Dual = FQuat(0.f, 0.f, 0.f, 0.f); // 합산을 위해 0으로 초기화

    float TotalWeight = 0.0f;
    bool bFirstInfluence = true;
    FQuat FirstRealQuat; // 첫 번째 유효한 영향의 Real 쿼터니언을 저장하기 위함

    // 각 인플루언스마다 가중치 곱해 더하기
    for (int i = 0; i < MAX_TOTAL_INFLUENCES; ++i)
    {
        float w = Src.InfluenceWeights[i];
        if (w <= KINDA_SMALL_NUMBER) continue;

        int32 bi = Src.InfluenceBones[i];
        FDualQuat CurrentBoneDQ = SkinDQs[bi]; // 원본 DQ 복사

        if (bFirstInfluence)
        {
            FirstRealQuat = CurrentBoneDQ.Real;
            BlendedDQ.Real.X = CurrentBoneDQ.Real.X * w;
            BlendedDQ.Real.Y = CurrentBoneDQ.Real.Y * w;
            BlendedDQ.Real.Z = CurrentBoneDQ.Real.Z * w;
            BlendedDQ.Real.W = CurrentBoneDQ.Real.W * w;

            BlendedDQ.Dual.X = CurrentBoneDQ.Dual.X * w;
            BlendedDQ.Dual.Y = CurrentBoneDQ.Dual.Y * w;
            BlendedDQ.Dual.Z = CurrentBoneDQ.Dual.Z * w;
            BlendedDQ.Dual.W = CurrentBoneDQ.Dual.W * w;
            bFirstInfluence = false;
        }
        else
        {
            // 현재 쿼터니언의 Real 파트가 FirstRealQuat과 다른 반구에 있다면 부호를 뒤집음
            // FQuat::DotProduct는 FQuat에 이미 정의되어 있다고 가정
            if (FQuat::DotProduct(FirstRealQuat, CurrentBoneDQ.Real) < 0.0f)
            {
                // DualQuaternion 전체의 부호를 변경
                CurrentBoneDQ.Real = CurrentBoneDQ.Real * -1.0f;
                CurrentBoneDQ.Dual = CurrentBoneDQ.Dual * -1.0f;
            }
            // 가중치를 적용하여 합산
            BlendedDQ.Real.X += CurrentBoneDQ.Real.X * w;
            BlendedDQ.Real.Y += CurrentBoneDQ.Real.Y * w;
            BlendedDQ.Real.Z += CurrentBoneDQ.Real.Z * w;
            BlendedDQ.Real.W += CurrentBoneDQ.Real.W * w;

            BlendedDQ.Dual.X += CurrentBoneDQ.Dual.X * w;
            BlendedDQ.Dual.Y += CurrentBoneDQ.Dual.Y * w;
            BlendedDQ.Dual.Z += CurrentBoneDQ.Dual.Z * w;
            BlendedDQ.Dual.W += CurrentBoneDQ.Dual.W * w;
        }
        TotalWeight += w;
    }

    if (TotalWeight <= KINDA_SMALL_NUMBER)
    {
        // 영향이 없으면 바인드포즈 그대로
        Out.X = Src.Position.X; Out.Y = Src.Position.Y; Out.Z = Src.Position.Z;
        FVector N = FVector(Src.TangentZ.X, Src.TangentZ.Y, Src.TangentZ.Z).GetSafeNormal();
        Out.NormalX = N.X; Out.NormalY = N.Y; Out.NormalZ = N.Z;
        FVector T = Src.TangentX.GetSafeNormal();
        Out.TangentX = T.X; Out.TangentY = T.Y; Out.TangentZ = T.Z;
        return;
    }

    // 가중치 합으로 나눔 (평균화). 아직 DQ 정규화는 아님.
    float InvTotalWeight = 1.0f / TotalWeight;
    BlendedDQ.Real = BlendedDQ.Real * InvTotalWeight;
    BlendedDQ.Dual = BlendedDQ.Dual * InvTotalWeight;

    // 최종 블렌딩된 듀얼 쿼터니언 정규화
    BlendedDQ.Normalize();

    // 위치 변환
    FVector SkinnedPos = BlendedDQ.TransformPosition(Src.Position);

    // 노말/탄젠트 변환
    // 참고: Src.TangentZ는 FVector4이지만, 회전만 적용할 때는 FVector로 다루는 것이 일반적.
    //       만약 TangentZ.W가 특별한 의미를 갖는다면 TransformFVector4가 맞으나,
    //       보통 노멀/탄젠트는 방향 벡터이므로 Real 파트로만 회전.
    //       원본 코드에서는 TransformFVector4를 사용했으므로 유지.
    FVector4 SkinnedN4 = BlendedDQ.TransformFVector4(Src.TangentZ); // Real 파트로만 회전
    FVector  SkinnedN(SkinnedN4.X, SkinnedN4.Y, SkinnedN4.Z);
    FVector  SkinnedT = BlendedDQ.TransformVector(Src.TangentX); // Real 파트로만 회전

    // 결과 저장
    Out.X = SkinnedPos.X;  Out.Y = SkinnedPos.Y;  Out.Z = SkinnedPos.Z;

    SkinnedN.Normalize(); // 안전하게 정규화
    Out.NormalX = SkinnedN.X;  Out.NormalY = SkinnedN.Y;  Out.NormalZ = SkinnedN.Z;

    SkinnedT.Normalize(); // 안전하게 정규화
    Out.TangentX = SkinnedT.X;  Out.TangentY = SkinnedT.Y;  Out.TangentZ = SkinnedT.Z; // SkinnedT.Z 오타 수정
}


#else
void FSkeletalMeshObjectCPUSkin::Update(USkinnedMeshComponent* InMeshComponent)
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
    FVector4 SkinnedNormal = FVector4(0, 0, 0, 0);
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

        if (BoneGlobalTransforms.Num() <= BoneIndex || InverseBindPose.Num() <= BoneIndex)
        {
            int a = 1;
            return;
        }

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
#endif 
