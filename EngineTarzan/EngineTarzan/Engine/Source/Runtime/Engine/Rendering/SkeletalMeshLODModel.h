#pragma once
#include "Container/Array.h"
#include "Math/Color.h"
#include "Math/Vector4.h"
#include "Define.h"

enum
{
    MAX_TEXCOORDS = 4,
    MAX_TOTAL_INFLUENCES = 4,
};

struct FSoftSkinVertex
{
    FVector Position;
    FLinearColor Color;                   // Vertex Color
    FVector4 TangentZ;              // Normal
    FVector TangentX;               // Tangent, U-Direction
    FVector TangentY;               // BiTangent, V-Direction

    FVector2D UVs[MAX_TEXCOORDS];   // Texture Coordinates

    uint8 InfluenceBones[MAX_TOTAL_INFLUENCES];     // 영향을 주는 Bone Index Bones
    float InfluenceWeights[MAX_TOTAL_INFLUENCES];   // 위에 해당하는 가중치 값

    uint32 MaterialIndex = 0; // 머티리얼 슬롯 인덱스

    //friend FArchive& operator<<(FArchive& Ar, FSoftSkinVertex& V);
};

// 섹션 정보 
struct FSkelMeshSection {
    uint16       MaterialIndex;     // 머티리얼 슬롯
    uint32       BaseIndex;         // 인덱스 버퍼 시작 위치
    uint32       NumTriangles;      // 섹션의 삼각형 개수
    uint32       BaseVertexIndex;   // 정점 버퍼 시작 위치
    TArray<int32> BoneMap;          // 이 섹션에서 사용되는 본 인덱스 목록
};

/** 에디터 내부에서 LOD 편집·쿠킹 로직(FSkeletalMeshBuilder)이 읽어들여 런타임용 FSkeletalMeshRenderData로 변환대상이 될 Raw Data */
class FSkeletalMeshLODModel
{
public:
    FWString ObjectName;         // LOD 모델 이름
    FString DisplayName;         // LOD 모델 표시 이름


    TArray<FSoftSkinVertex>  Vertices;           // 정점 데이터
    TArray<uint32>           Indices;            // 인덱스 데이터
    TArray<uint32>           Faces;              // 폴리곤 인덱스

    TArray<int32>            RequiredBones;      // 렌더링에 필요한 본 인덱스
    TArray<FMatrix>          RefBasesInvMatrix;  // 역바인드 포즈 행렬

    uint32 NumVertices;
    uint32 NumTexCoords;

    TArray<FSkelMeshSection> Sections; // 머티리얼별 인덱스 범위와 섹션 단위 본 맵

    TArray<FStaticMaterial> Materials; // 머티리얼 슬롯
};

