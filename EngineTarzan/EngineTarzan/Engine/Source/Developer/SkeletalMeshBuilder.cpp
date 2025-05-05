#include "SkeletalMeshBuilder.h"

#include "Engine/Asset/SkeletalMeshAsset.h"

void FSkeletalMeshBuilder::BuildRenderData(
    const FSkeletalMeshLODModel& InLODModel,
    FSkeletalMeshRenderData& OutRenderData)
{
    ConvertLODModelToRenderData(InLODModel, OutRenderData);
}

/**
 * LODModel의 Raw SoftSkin 데이터를 런타임용 RenderData로 변환
 */
void FSkeletalMeshBuilder::ConvertLODModelToRenderData(
    const FSkeletalMeshLODModel& LODModel,
    FSkeletalMeshRenderData& OutRenderData)
{
    // 메타데이터
    OutRenderData.ObjectName = LODModel.ObjectName;
    OutRenderData.DisplayName = LODModel.DisplayName;

    // 정점 변환
    OutRenderData.Vertices.Empty();
    OutRenderData.Vertices.Reserve(LODModel.Vertices.Num());
    for (const FSoftSkinVertex& Src : LODModel.Vertices)
    {
        FSkeletalMeshVertex Dst;
        // 위치
        Dst.X = Src.Position.X;
        Dst.Y = Src.Position.Y;
        Dst.Z = Src.Position.Z;
        // 컬러
        Dst.R = Src.Color.R;
        Dst.G = Src.Color.G;
        Dst.B = Src.Color.B;
        Dst.A = Src.Color.A;
        // 노말
        Dst.NormalX = Src.TangentZ.X;
        Dst.NormalY = Src.TangentZ.Y;
        Dst.NormalZ = Src.TangentZ.Z;
        // 탄젠트(교차곱으로 W 사인 계산)
        FVector  T = Src.TangentX;
        FVector  Bn = Src.TangentY;
        FVector  N = FVector(Src.TangentZ.X, Src.TangentZ.Y, Src.TangentZ.Z);
        Dst.TangentX = T.X;
        Dst.TangentY = T.Y;
        Dst.TangentZ = T.Z;
        float Sign = (FVector::CrossProduct(N, T) | Bn) < 0.f ? -1.f : 1.f;
        Dst.TangentW = Sign;
        // UV (첫번째 채널)
        Dst.U = Src.UVs[0].X;
        Dst.V = Src.UVs[0].Y;

        OutRenderData.Vertices.Add(Dst);
    }

    // 2) 인덱스 & 머티리얼 서브셋 구성
    OutRenderData.Indices.Empty();
    OutRenderData.MaterialSubsets.Empty();

    for (const FSkelMeshSection& Sec : LODModel.Sections)
    {
        FMaterialSubset Subset;
        Subset.MaterialIndex = Sec.MaterialIndex;        // 머티리얼 슬롯
        Subset.IndexStart = OutRenderData.Indices.Num(); // 지금까지 들어간 인덱스 수
        Subset.IndexCount = Sec.NumTriangles * 3;       // 섹션의 인덱스 개수

        // Sec.BaseIndex 에서 시작해, Sec.NumTriangles만큼 3개씩 복사
        for (uint32 tri = 0; tri < Sec.NumTriangles; ++tri)
        {
            uint32 base = Sec.BaseIndex + tri * 3;
            OutRenderData.Indices.Add(LODModel.Indices[base + 0]);
            OutRenderData.Indices.Add(LODModel.Indices[base + 1]);
            OutRenderData.Indices.Add(LODModel.Indices[base + 2]);
        }

        OutRenderData.MaterialSubsets.Add(Subset);
    }


    // 바운딩 박스 계산
    if (OutRenderData.Vertices.Num() > 0)
    {
        FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
        FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        for (const FSkeletalMeshVertex& V : OutRenderData.Vertices)
        {
            Min.X = FMath::Min(Min.X, V.X);
            Min.Y = FMath::Min(Min.Y, V.Y);
            Min.Z = FMath::Min(Min.Z, V.Z);
            Max.X = FMath::Max(Max.X, V.X);
            Max.Y = FMath::Max(Max.Y, V.Y);
            Max.Z = FMath::Max(Max.Z, V.Z);
        }
        OutRenderData.BoundingBoxMin = Min;
        OutRenderData.BoundingBoxMax = Max;
    }

    // 바인드 포즈 매트릭스 복사
    OutRenderData.ReferenceToLocalMatrices.Empty();
    OutRenderData.ReferenceToLocalMatricesInverse.Empty();
    OutRenderData.ReferenceToLocalMatricesInverse = LODModel.RefBasesInvMatrix;
    OutRenderData.ReferenceToLocalMatrices.Reserve(LODModel.RefBasesInvMatrix.Num());
    for (const FMatrix& Inv : LODModel.RefBasesInvMatrix)
    {
        OutRenderData.ReferenceToLocalMatrices.Add(FMatrix::Inverse(Inv));
    }
}
