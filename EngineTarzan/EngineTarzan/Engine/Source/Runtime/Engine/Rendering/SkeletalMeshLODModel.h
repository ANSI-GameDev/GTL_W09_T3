#pragma once
#include "Container/Array.h"
#include "Math/Color.h"
#include "Math/Vector4.h"

enum
{
    MAX_TEXCOORDS = 4,
    MAX_TOTAL_INFLUENCES = 4,
};

struct FSoftSkinVertex
{
    FVector Position;
    FVector TangentX;               // Tangent, U-Direction
    FVector TangentY;               // BiTangent, V-Direction
    FVector4 TangentZ;              // Normal

    FVector2D UVs[MAX_TEXCOORDS];   // Texture Coordinates
    FColor Color;                   // Vertex Color

    uint8 InfluenceBones[MAX_TOTAL_INFLUENCES];     // 영향을 주는 Bone Index Bones
    float InfluenceWeights[MAX_TOTAL_INFLUENCES];   // 위에 해당하는 가중치 값

    //friend FArchive& operator<<(FArchive& Ar, FSoftSkinVertex& V);
};

/** 에디터 내부에서 LOD 편집·쿠킹 로직(FSkeletalMeshBuilder)이 읽어들여 런타임용 FSkeletalMeshRenderData로 변환대상이 될 Raw Data */
class FSkeletalMeshLODModel
{
public:
    TArray<FSoftSkinVertex>  Vertices;           // 정점 데이터
    TArray<uint32>           Indices;            // 인덱스 데이터
    TArray<uint32>           Faces;              // 폴리곤 인덱스

    TArray<int32>            RequiredBones;      // 렌더링에 필요한 본 인덱스
    TArray<FMatrix>          RefBasesInvMatrix;  // 역바인드 포즈 행렬

    uint32 NumVertices;
    uint32 NumTexCoords;


};

