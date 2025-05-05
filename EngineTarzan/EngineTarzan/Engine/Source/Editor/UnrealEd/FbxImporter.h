#pragma once
#include <fbxsdk.h>

#include "Define.h"
#include "Container/Map.h"
#include "HAL/PlatformType.h"

struct FReferenceSkeleton;

class FSkeletalMeshLODModel;
class FString;
class USkeletalMesh;
class FMatrix;

class FFbxImporter
{

public:
    /**
     * 파일 경로만 주면 내부에서 씬 로드→본/LOD 파싱까지 한 번에 처리
     * @param InFilePath    FBX 파일 경로
     * @param LodModel      채워넣을 LOD 모델
     * @param OutRefSkeleton (선택) 본 계층도 같이 채우려면 포인터 전달
     */
    static bool ParseSkeletalMeshLODModel(
        const FString& InFilePath,
        FSkeletalMeshLODModel& LodModel,
        FReferenceSkeleton* OutRefSkeleton = nullptr
    );

    static bool ParseSkeletalMeshLODModel(FbxMesh* Mesh, FSkeletalMeshLODModel& LodModel, uint32 GlobalIdxCtr, const FString& InFilePath);

    static bool ParseFbxMaterialTextures(FbxSurfaceMaterial* InMaterial, FStaticMaterial& OutMat, const FString& BasePath);

    static bool ParseReferenceSkeleton(const FString& InFilePath, FReferenceSkeleton& OutRefSkeleton);

    // 헬퍼: 씬 전체를 언리얼 좌표계 + 단위로 변환
    static void ConvertSceneToUnreal(FbxScene* Scene);

    // 헬퍼: FBX 노드 재귀 순회하여 첫 번째 메시 찾기
    static FbxMesh* FindFirstMeshInScene(FbxScene* Scene);

    static USkeletalMesh* ImportSkeletalMesh(const FString& InFilePath);

private:
    // FBX 노드 재귀 순회하여 Skeleton 노드만 RefSkeleton에 추가
    static void BuildReferenceSkeleton(FbxNode* Node, FReferenceSkeleton& OutRefSkeleton, uint32 ParentIndex, int32 Depth);

private:

    /** FBX 노드 → 본 인덱스 맵 (ParseReferenceSkeleton 이후 채워져 있어야 함) */
    static TMap<FbxNode*, int32> NodeToBoneIndex;

    /** FBX→언리얼 축·단위 보정 행렬 (GetJointPostConversionMatrix()) */
    static FbxAMatrix JointPostConvert;

    /** FBX 행렬 → 엔진 FMatrix 변환 헬퍼 */
    static FMatrix ConvertFbxAMatrix(const FbxAMatrix& M);
};

