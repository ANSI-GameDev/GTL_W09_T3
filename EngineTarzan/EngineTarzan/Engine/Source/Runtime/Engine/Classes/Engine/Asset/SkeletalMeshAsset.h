#pragma once

#include "Define.h"
#include "Hal/PlatformType.h"
#include "Container/Array.h"
#include "Rendering/SkeletalMeshLODModel.h"


//struct FSkeletalMeshVertex
//{
//    float X, Y, Z;    // Position
//    float R, G, B, A; // Color
//    float NormalX, NormalY, NormalZ;
//    float TangentX, TangentY, TangentZ, TangentW;
//    float U = 0, V = 0;
//    uint32 MaterialIndex;
//};

struct FSkeletalMeshRenderData
{
    FWString ObjectName;
    FString DisplayName;

    TArray<FSoftSkinVertex>  Vertices;           // 정점 데이터
    TArray<UINT> Indices;

    TArray<FObjMaterialInfo> Materials;
    TArray<FMaterialSubset> MaterialSubsets;

    FVector BoundingBoxMin;
    FVector BoundingBoxMax;

    // ----------------------- // 여기서부터 FBX Import 관련

    TArray<FMatrix> ReferenceToLocalMatrices;        // Bind Pose Matrix
    TArray<FMatrix> ReferenceToLocalMatricesInverse; // Bind Pose Inv Matrix
};
