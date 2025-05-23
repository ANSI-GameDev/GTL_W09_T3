#include <windows.h>

#include "FbxImporter.h"
#include "Define.h"
#include "ReferenceSkeleton.h"
#include "Components/Material/Material.h"
#include "Engine/FObjLoader.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "UObject/ObjectFactory.h"

TMap<FbxNode*, int32> FFbxImporter::NodeToBoneIndex;
FbxAMatrix            FFbxImporter::JointPostConvert;

/* Find All Mesh per Node via DFS */
void CollectAllMeshes(FbxNode* Node, TArray<FbxMesh*>& Out)
{
    if (auto* M = Node->GetMesh())
    {
        Out.Add(M);
    }
    for (int i = 0; i < Node->GetChildCount(); ++i)
    {
        CollectAllMeshes(Node->GetChild(i), Out);
    }
}

// --- Reference Skeleton 파싱 (파일 기반) ---
bool FFbxImporter::ParseReferenceSkeleton(const FString& InFilePath, FReferenceSkeleton& OutRefSkeleton)
{
    // 1) 매핑 테이블 초기화
    NodeToBoneIndex.Empty();

    // 2) FBX SDK 초기화
    FbxManager* SdkMgr = FbxManager::Create();
    FbxIOSettings* IOSettings = FbxIOSettings::Create(SdkMgr, IOSROOT);
    SdkMgr->SetIOSettings(IOSettings);

    // 3) Importer 생성
    FbxImporter* Importer = FbxImporter::Create(SdkMgr, TEXT(""));
    if (!Importer->Initialize(*InFilePath, -1, SdkMgr->GetIOSettings()))
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to initialize FBX Importer for %s"), *InFilePath);
        Importer->Destroy();
        SdkMgr->Destroy();
        return false;
    }

    // 4) Scene 생성 및 임포트
    FbxScene* Scene = FbxScene::Create(SdkMgr, TEXT("Scene"));
    Importer->Import(Scene);
    Importer->Destroy();

    // 5) Reference Skeleton 빌드
    BuildReferenceSkeleton(Scene->GetRootNode(), OutRefSkeleton, INDEX_NONE, 0);

    // 6) SDK 정리
    Scene->Destroy();
    SdkMgr->Destroy();

    // 7) 결과 로깅
    bool bSuccess = (OutRefSkeleton.GetNumBones() > 0);
    UE_LOG(LogLevel::Error, TEXT("ReferenceSkeleton parsing %s"), bSuccess ? TEXT("succeeded") : TEXT("failed"));
    return bSuccess;
}

void FFbxImporter::ConvertSceneToUnreal(FbxScene* Scene)
{
    // 언리얼 축계: Z-Up, X-Forward, LH
    FbxAxisSystem UnrealAxis(
        FbxAxisSystem::eZAxis,
        FbxAxisSystem::eParityEven,
        FbxAxisSystem::eLeftHanded
    );

    // 먼저 FBX 씬에 설정된 축계 → 언리얼 축계로
    Scene->GetGlobalSettings().GetAxisSystem().ConvertScene(Scene);
    UnrealAxis.DeepConvertScene(Scene);

    Scene->GetGlobalSettings().SetAxisSystem(UnrealAxis);
    // [Unused] position 에 scale 값 곱하여 구함 - 단위: FBX 기본(cm) → 언리얼 기본(m) (1/100)
    //FbxSystemUnit::cm.ConvertScene(Scene);
}

// --- 추후 구현 예정 ---
USkeletalMesh* FFbxImporter::ImportSkeletalMesh(const FString& InFilePath)
{
    UE_LOG(LogLevel::Warning, TEXT("ImportSkeletalMesh not yet implemented for %s"), *InFilePath);
    return nullptr;
}

// --- Reference Skeleton 빌드 ---
void FFbxImporter::BuildReferenceSkeleton(FbxNode* Node, FReferenceSkeleton& OutRefSkeleton, uint32 ParentIndex, int32 Depth)
{
    if (!Node) return;

    // 최상위 호출 시 매핑 초기화
    if (Depth == 0)
    {
        NodeToBoneIndex.Empty();
    }

    // 본 어트리뷰트 체크
    FbxNodeAttribute* Attr = Node->GetNodeAttribute();
    int32 ThisParent = ParentIndex;
    if (Attr && Attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
    {
        FName BoneName(Node->GetName());
        FbxAMatrix Local = Node->EvaluateLocalTransform();
        FTransform LocalBonePose(
            FVector(Local.GetT()[0], Local.GetT()[1], Local.GetT()[2]) * (1.f / 100.f),
            FQuat(Local.GetQ()[0], Local.GetQ()[1], Local.GetQ()[2], Local.GetQ()[3]),
            FVector(Local.GetS()[0], Local.GetS()[1], Local.GetS()[2])
        );

        FbxAMatrix Global = Node->EvaluateGlobalTransform();
        FTransform GlobalBonePose(
            FVector(Global.GetT()[0], Global.GetT()[1], Global.GetT()[2]) * (1.f / 100.f),
            FQuat(Global.GetQ()[0], Global.GetQ()[1], Global.GetQ()[2], Global.GetQ()[3]),
            FVector(Global.GetS()[0], Global.GetS()[1], Global.GetS()[2])
        );

        // OutRefSkeleton 에 본 추가
        ThisParent = OutRefSkeleton.AddBone(BoneName, ParentIndex, LocalBonePose);
        OutRefSkeleton.RefBonePose.Add(GlobalBonePose);
        NodeToBoneIndex.Add(Node, ThisParent);

        // 트리 로그
        FString Prefix;
        if (Depth > 0)
        {
            Prefix = TEXT("  ");
            for (int32 i = 1; i < Depth; ++i) Prefix += TEXT("  ");
            Prefix += TEXT("|__");
        }
        UE_LOG(LogLevel::Error, TEXT("%sBone: %s (ParentIndex: %d, MyIndex: %d)"), *Prefix, *BoneName.ToString(), ParentIndex, ThisParent);
    }

    // 자식 노드 재귀 호출
    for (int32 i = 0; i < Node->GetChildCount(); ++i)
    {
        BuildReferenceSkeleton(Node->GetChild(i), OutRefSkeleton, ThisParent, Depth + 1);
    }
}

// --- 행렬 변환 유틸 ---
FMatrix FFbxImporter::ConvertFbxAMatrix(const FbxAMatrix& M)
{
    FMatrix Out;
    const double* Src = reinterpret_cast<const double*>(&M);
    float* Dst = reinterpret_cast<float*>(&Out);
    for (int i = 0; i < 16; ++i) Dst[i] = (float)Src[i];
    return Out;
}

FbxMesh* FFbxImporter::FindFirstMeshInScene(FbxScene* Scene)
{
    if (!Scene->GetRootNode()) return nullptr;
    TArray<FbxNode*> q; q.Add(Scene->GetRootNode());
    for (int i = 0; i < q.Num(); ++i)
    {
        FbxNode* n = q[i];
        if (n->GetMesh()) return n->GetMesh();
        for (int c = 0; c < n->GetChildCount(); ++c)
            q.Add(n->GetChild(c));
    }
    return nullptr;
}

// --- LODModel 파일 파싱 및 본 계층 적용 ---
bool FFbxImporter::ParseSkeletalMeshLODModel(
    const FString& InFilePath,
    FSkeletalMeshLODModel& LodModel,
    FReferenceSkeleton* OutRefSkeleton
)
{
    FbxManager* SdkMgr = FbxManager::Create();
    FbxIOSettings* IOS = FbxIOSettings::Create(SdkMgr, IOSROOT);
    SdkMgr->SetIOSettings(IOS);
    FbxImporter* Importer = FbxImporter::Create(SdkMgr, TEXT(""));
    if (!Importer->Initialize(*InFilePath, -1, SdkMgr->GetIOSettings()))
    {
        return false;
    }

    // Scene 생성 및 임포트
    FbxScene* Scene = FbxScene::Create(SdkMgr, TEXT("Scene"));
    Importer->Import(Scene);
    Importer->Destroy();

    // 씬 전체 Triangulate
    
    ConvertSceneToUnreal(Scene);
    FbxGeometryConverter geomConverter(SdkMgr);
    geomConverter.Triangulate(Scene, /*replace=*/true); // true: 기존 폴리곤은 모두 삭제하고, 결과 메시만 남김

    /** Reference Skeleton의 Bone 정보 업데이트 및 역바인드 행렬 저장 */
    if (OutRefSkeleton)
    {
        BuildReferenceSkeleton(Scene->GetRootNode(), *OutRefSkeleton, INDEX_NONE, 0);
        UE_LOG(LogLevel::Error, TEXT("ReferenceSkeleton built from existing Scene"));
        OutRefSkeleton->InitializeInverseBindPose();
    }

    // Clear existing LODModel data and initialize global index counter
    LodModel.Vertices.Empty();
    LodModel.Indices.Empty();
    LodModel.Faces.Empty();
    LodModel.Sections.Empty();
    LodModel.RequiredBones.Empty();
    LodModel.RefBasesInvMatrix.Empty();

    uint32 GlobalIdxCtr = 0;
    uint32 OverallIndexOffset = 0; // 전체 인덱스 오프셋

    LodModel.FilePath = InFilePath.ToWideString().substr(0,
        InFilePath.ToWideString().find_last_of(L"\\/") + 1);
    LodModel.ObjectName = InFilePath.ToWideString();
    std::wstring wideName = LodModel.ObjectName.substr(InFilePath.ToWideString().find_last_of(L"\\/") + 1);;
    std::string fileName(wideName.begin(), wideName.end());

    // 마지막 '.'을 찾아 확장자를 제거
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        LodModel.DisplayName = fileName.substr(0, dotPos);
    }
    else
    {
        LodModel.DisplayName = fileName;
    }
    

    // 씬 전체에서 DFS로 Mesh를 Node 순회하며 수집 후 각각 처리
    TArray<FbxMesh*> MeshList;
    CollectAllMeshes(Scene->GetRootNode(), MeshList);
    bool bResult = false;
    for (FbxMesh* ChildMesh : MeshList)
    {
        bResult |= ParseSkeletalMeshLODModel(ChildMesh, LodModel, GlobalIdxCtr, InFilePath, OverallIndexOffset);
        OverallIndexOffset = LodModel.Indices.Num();
    }

    // 6) 정리
    Scene->Destroy();
    SdkMgr->Destroy();
    return bResult;
}

bool FFbxImporter::ParseSkeletalMeshLODModel(FbxMesh* Mesh, FSkeletalMeshLODModel& LodModel, uint32& GlobalIdxCtr, const FString& InFilePath, uint32& OverallIndexOffset)
{
    if (!Mesh) return false;

    // 1) FBX 노드 머티리얼 파싱
    {
        FbxNode* Node = Mesh->GetNode();
        int32 MatCount = Node->GetMaterialCount();
        FString BasePath = FString(L"Contents/FBX/");
        for (int32 i = 0; i < MatCount; ++i)
        {
            if (auto* Mat = Node->GetMaterial(i))
            {
                FStaticMaterial OutMat;
                OutMat.Material = FObjectFactory::ConstructObject<UMaterial>(nullptr);
                OutMat.MaterialSlotName = FName(Mat->GetName());
                ParseFbxMaterialTextures(Mat, OutMat, BasePath);
                LodModel.Materials.Add(OutMat);
            }
        }
    }

    // 추가될 첫 정점 인덱스
    const uint32 MeshStartVert = LodModel.Vertices.Num();

    // 0) 노말·탄젠트 생성 및 레이어 찾기
    if (Mesh->GetLayerCount() == 0 || !Mesh->GetLayer(0)->GetNormals())
    {
        Mesh->InitNormals();
        Mesh->GenerateNormals(true);
    }
    if (!Mesh->GetLayer(0)->GetTangents())
    {
        Mesh->InitTangents();
        Mesh->GenerateTangentsData(0, true, true);
    }
    if (!Mesh->GetLayer(0)->GetBinormals())
    {
        Mesh->InitBinormals();
        Mesh->CreateElementBinormal();
    }

    // 레이어 검색
    FbxLayerElementNormal* normalLayer = nullptr;
    FbxLayerElementTangent* tangentLayer = nullptr;
    FbxLayerElementBinormal* binormalLayer = nullptr;
    FbxLayerElementVertexColor* colorLayer = nullptr;
    FbxLayerElementMaterial* materialLayer = nullptr;
    for (int li = 0; li < Mesh->GetLayerCount(); ++li)
    {
        auto* layer = Mesh->GetLayer(li);
        if (!normalLayer && layer->GetNormals())      normalLayer = layer->GetNormals();
        if (!tangentLayer && layer->GetTangents())     tangentLayer = layer->GetTangents();
        if (!binormalLayer && layer->GetBinormals())    binormalLayer = layer->GetBinormals();
        if (!colorLayer && layer->GetVertexColors()) colorLayer = layer->GetVertexColors();
        if (!materialLayer && layer->GetMaterials())    materialLayer = layer->GetMaterials();
    }

    // UV 레이어 검색
    int32 uvCount = FMath::Min(Mesh->GetElementUVCount(), (int32)MAX_TEXCOORDS);
    TArray<FbxGeometryElementUV*> uvLayers;
    for (int32 ui = 0; ui < uvCount; ++ui)
    {
        uvLayers.Add(Mesh->GetElementUV(ui));
    }

    // 1) Geo 트랜스폼 & NormalMat 계산
    FbxNode* Node = Mesh->GetNode();
    FbxAMatrix Geo;
    Geo.SetIdentity();
    Geo.SetT(Node->GetGeometricTranslation(FbxNode::eSourcePivot));
    Geo.SetR(Node->GetGeometricRotation(FbxNode::eSourcePivot));
    Geo.SetS(Node->GetGeometricScaling(FbxNode::eSourcePivot));

    FbxAMatrix NormalMat = Geo.Inverse().Transpose();

    // 헬퍼 함수: 매핑 모드에 따른 rawIdx 계산
    auto GetRawIndex = [&](auto* elem, int32 cp, int32 idxCtr, int32 p) -> int32
        {
            switch (elem->GetMappingMode())
            {
            case FbxGeometryElement::eByControlPoint:  return cp;
            case FbxGeometryElement::eByPolygonVertex: return idxCtr;
            case FbxGeometryElement::eByPolygon:       return p;
            case FbxGeometryElement::eAllSame:         return 0;
            default:                                   return idxCtr;
            }
        };
    // 헬퍼 함수: 참조 모드에 따른 finalIdx 계산
    auto GetFinalIndex = [&](auto* elem, int rawIdx)
        {
            switch (elem->GetReferenceMode())
            {
            case FbxLayerElement::eDirect:
                return rawIdx;
            case FbxLayerElement::eIndex:
            case FbxLayerElement::eIndexToDirect:
                return elem->GetIndexArray().GetAt(rawIdx);
            default:
                return rawIdx;
            }
        };
    // --- 섹션 파싱: 재질별 폴리곤 그룹화 ---
    if (materialLayer)
    {
        // 메시별로 분리된 sectionMap
        TMap<int32, FSkelMeshSection> sectionMap;
        int32 polyCount = Mesh->GetPolygonCount();

        // eByPolygonVertex 대응용 "폴리곤 정점 순서" 카운터
        int32 groupingIdxCtr = 0;

        // (1) 각 폴리곤마다 올바른 rawIdx 계산
        for (int32 p = 0; p < polyCount; ++p)
        {
            // 대표 컨트롤 포인트로 폴리곤의 첫 번째 정점 인덱스 사용
            int32 cp = Mesh->GetPolygonVertex(p, 0);
            int rawIdx = GetRawIndex(materialLayer, cp, groupingIdxCtr, p);
            int32 matIdx = GetFinalIndex(materialLayer, rawIdx);

            // 새로운 섹션이면 생성
            FSkelMeshSection* sec = sectionMap.Find(matIdx);
            if (!sec)
            {
                FSkelMeshSection newSec;
                newSec.MaterialIndex = matIdx;
                newSec.BaseIndex = 0;
                newSec.NumTriangles = 0;
                newSec.BaseVertexIndex = 0;
                sectionMap.Add(matIdx, newSec);
                sec = sectionMap.Find(matIdx);
            }

            // 삼각형 개수 누적
            sec->NumTriangles += (Mesh->GetPolygonSize(p) - 2);

            // 다음 폴리곤정점을 위해 카운터 증가
            groupingIdxCtr += Mesh->GetPolygonSize(p);
        }

        // TMap 순회 순서가 예측 가능하도록 키 정렬
        TArray<int32> sortedKeys;
        for (auto& [key, value] : sectionMap)
        {
            sortedKeys.Add(key);
        }
        sortedKeys.Sort();

        // BaseIndex/BaseVertexIndex 계산 및 LodModel.Sections 추가
        uint32 runningTri = 0;
        for (int32 matKey : sortedKeys)
        {
            FSkelMeshSection& sec = sectionMap[matKey];
            sec.BaseIndex = OverallIndexOffset + (runningTri * 3);  // 인덱스버퍼 오프셋
            sec.BaseVertexIndex = MeshStartVert;                    // 정점버퍼 오프셋
            runningTri += sec.NumTriangles;
            LodModel.Sections.Add(sec);
        }
    }

    // 2) 스킨 클러스터 처리
    int32 CPCount = Mesh->GetControlPointsCount();
    std::vector<std::vector<std::pair<int32, double>>> Influences(CPCount);
    TMap<int32, FbxAMatrix> BindPoseMap;
    for (int di = 0; di < Mesh->GetDeformerCount(FbxDeformer::eSkin); ++di)
    {
        auto* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(di, FbxDeformer::eSkin));
        for (int ci = 0; ci < Skin->GetClusterCount(); ++ci)
        {
            FbxCluster* Cluster = Skin->GetCluster(ci);
            FbxAMatrix Mlink, Minitial;
            Cluster->GetTransformLinkMatrix(Mlink);     // 메시에 바인드된 (모델 공간) 변환
            Cluster->GetTransformMatrix(Minitial);      // Bone의 바인드된 (모델 공간) 변환
            FbxAMatrix Bind = Minitial.Inverse() * Mlink;  // 모델 공간 좌표 -> 메시 로컬 공간 -> 본 로컬 공간

            FbxNode* LinkNode = Cluster->GetLink();
            int32* FoundIdx = NodeToBoneIndex.Find(LinkNode);
            if (!FoundIdx)
            {
                UE_LOG(LogLevel::Warning, TEXT("Unmapped bone: %s"), *FString(LinkNode->GetName()));
                continue;
            }
            int32 BoneIdx = *FoundIdx;
            if (!BindPoseMap.Contains(BoneIdx))
            {
                BindPoseMap.Add(BoneIdx, Bind.Inverse()); // RefInverseBindPose에 추가할 행렬
                LodModel.RequiredBones.Add(BoneIdx);
            }
            UE_LOG(LogLevel::Error, TEXT("Cluster %d -> Bone '%s' idx=%d"), ci, *FString(LinkNode->GetName()), BoneIdx);

            int cnt = Cluster->GetControlPointIndicesCount();
            int* cpis = Cluster->GetControlPointIndices();
            double* wts = Cluster->GetControlPointWeights();
            for (int k = 0; k < cnt; ++k)
                Influences[cpis[k]].emplace_back(BoneIdx, wts[k]);
        }
    }

    // 3) RefBasesInvMatrix 채우기
    LodModel.RefBasesInvMatrix.Empty();
    for (int32 Bi : LodModel.RequiredBones)
    {
        LodModel.RefBasesInvMatrix.Add(ConvertFbxAMatrix(BindPoseMap[Bi]));
    }
    constexpr float UnitScale = 1.0f / 100.0f;
    uint32 idxCtr = 0; // 폴리곤 내 정점 카운터 (GetRawIndex에서 eByPolygonVertex 모드 시 사용될 수 있음)
    int32 polyCount = Mesh->GetPolygonCount();
    for (int32 p = 0; p < polyCount; ++p) // 각 폴리곤(삼각형) 순회
    {
        // 현재 폴리곤(삼각형)의 Material Index 계산 
        int32 matIdx = 0; // 기본값 또는 재질 정보 없을 때 사용
        if (materialLayer)
        {
            // materialLayer의 MappingMode 확인
            FbxLayerElement::EMappingMode mappingMode = materialLayer->GetMappingMode();
            int rawIdx = 0;

            if (mappingMode == FbxLayerElement::eByPolygon) { // 가장 일반적 : 폴리곤 인덱스 사용
                rawIdx = GetRawIndex(materialLayer, 0, 0, p);
            }
            else if (mappingMode == FbxLayerElement::eAllSame) {
                // 모든 폴리곤이 같은 재질 사용
                rawIdx = GetRawIndex(materialLayer, 0, 0, 0); // 0 사용
            }
            else {
                // eByControlPoint 또는 eByPolygonVertex는 여기서 직접 매핑하기 복잡할 수 있음.
                // FBX 임포터는 보통 이런 경우 데이터를 재구성하거나 eByPolygon으로 변환하는 것을 권장.
                // 여기서는 일단 eByPolygon 기준으로 처리 시도하고 경고 로그.
                UE_LOG(LogLevel::Warning, TEXT("Material mapping mode %d might not assign correct   per-vertex material index directly here. Assuming eByPolygon for polygon %d."), (int)mappingMode, p);
                rawIdx = GetRawIndex(materialLayer, 0, 0, p); // Fallback 시도
            }
            matIdx = GetFinalIndex(materialLayer, rawIdx);
        }

        // Triangulate 후에는 폴리곤 크기는 항상 3이어야 하므로 Error 로그 출력
        int32 polySize = Mesh->GetPolygonSize(p);
        if (polySize != 3) {
            UE_LOG(LogLevel::Error, TEXT("Polygon %d is not a triangle (size=%d) after expected     triangulation. Skipping."), p, polySize);
            // idxCtr을 정확히 증가시켜야 할 수 있음 (eByPolygonVertex 경우 대비)
            // 하지만 에러 상황이므로 건너뛰는 것이 안전할 수 있음.
            // idxCtr += polySize;
            continue;
        }

        // 원본 폴리곤 인덱스를 Faces 배열에 추가 (삼각형 당 하나)
        LodModel.Faces.Add(p);

        // 삼각형의 각 정점(0, 1, 2) 순회
        for (int f = 0; f < 3; ++f)
        {
            // cp = 컨트롤 포인트 인덱스
            int32 cp = Mesh->GetPolygonVertex(p, f);
            FSoftSkinVertex V{}; // 새 정점 데이터 생성

            // --- Position ---
            auto P = Geo.MultT(Mesh->GetControlPoints()[cp]);
            V.Position = FVector((float)P[0], (float)P[1], (float)P[2]) * UnitScale;

            // MaterialIndex
            V.MaterialIndex = static_cast<uint32>(matIdx);

            // UVs 
            for (int32 u = 0; u < uvLayers.Num(); ++u)
            {
                auto* uvElem = uvLayers[u];
                // GetRawIndex 호출 시 idxCtr 전달 (eByPolygonVertex 대비)
                int rawIdx = GetRawIndex(uvElem, cp, idxCtr, p);
                int finalIdx = GetFinalIndex(uvElem, rawIdx);
                // Index 유효성 검사 추가 가능 (선택 사항)
                if (finalIdx >= 0 && finalIdx < uvElem->GetDirectArray().GetCount()) {
                    auto uv = uvElem->GetDirectArray().GetAt(finalIdx);
                    V.UVs[u] = FVector2D((float)uv[0], 1.f - (float)uv[1]); // Y좌표 반전
                }
                else {
                    UE_LOG(LogLevel::Warning, TEXT("Invalid final UV index %d for vertex (cp=%d, idxCtr=%u, p=%d)"), finalIdx, cp, idxCtr, p);
                    V.UVs[u] = FVector2D(0.f, 0.f); // 기본값
                }
            }

            // Color
            if (colorLayer)
            {
                int rawIdx = GetRawIndex(colorLayer, cp, idxCtr, p);
                int finalIdx = GetFinalIndex(colorLayer, rawIdx);
                if (finalIdx >= 0 && finalIdx < colorLayer->GetDirectArray().GetCount()) {
                    auto c = colorLayer->GetDirectArray().GetAt(finalIdx);
                    V.Color = FColor((uint8)(c.mRed * 255),
                        (uint8)(c.mGreen * 255),
                        (uint8)(c.mBlue * 255),
                        (uint8)(c.mAlpha * 255));
                }
                else {
                    UE_LOG(LogLevel::Warning, TEXT("Invalid final Color index %d for vertex (cp=%d, idxCtr=%u, p=%d)"), finalIdx, cp, idxCtr, p);
                    V.Color = FColor(255, 255, 255, 255); // 기본 흰색
                }
            }
            else {
                V.Color = FColor(255, 255, 255, 255); // Color Layer 없으면 기본 흰색
            }

            // Normal
            if (normalLayer)
            {
                int rawIdx = GetRawIndex(normalLayer, cp, idxCtr, p);
                int finalIdx = GetFinalIndex(normalLayer, rawIdx);
                if (finalIdx >= 0 && finalIdx < normalLayer->GetDirectArray().GetCount()) {
                    FbxVector4 n = normalLayer->GetDirectArray().GetAt(finalIdx);
                    n.Normalize(); // 정규화 확인
                    n = NormalMat.MultT(n); // 변환
                    n.Normalize(); // 변환 후 다시 정규화
                    V.TangentZ = FVector4((float)n[0], (float)n[1], (float)n[2], 0); // Z-up 변환
                }
                else {
                    UE_LOG(LogLevel::Warning, TEXT("Invalid final Normal index %d for vertex (cp=%d, idxCtr=%u, p=%d)"), finalIdx, cp, idxCtr, p);
                    V.TangentZ = FVector4(0.f, 0.f, 1.f, 0); // 기본 Z 방향
                }
            }
            else {
                V.TangentZ = FVector4(0.f, 0.f, 1.f, 0); // Normal Layer 없으면 기본 Z 방향
            }


            // Tangent
            if (tangentLayer)
            {
                int rawIdx = GetRawIndex(tangentLayer, cp, idxCtr, p);
                int finalIdx = GetFinalIndex(tangentLayer, rawIdx);
                if (finalIdx >= 0 && finalIdx < tangentLayer->GetDirectArray().GetCount()) {
                    FbxVector4 t = tangentLayer->GetDirectArray().GetAt(finalIdx);
                    t.Normalize();
                    t = NormalMat.MultT(t);
                    t.Normalize();
                    V.TangentX = FVector((float)t[0], (float)t[1], (float)t[2]); // Z-up 변환
                }
                else {
                    UE_LOG(LogLevel::Warning, TEXT("Invalid final Tangent index %d for vertex (cp=%d, idxCtr=%u, p=%d)"), finalIdx, cp, idxCtr, p);
                    V.TangentX = FVector(1.f, 0.f, 0.f); // 기본 X 방향
                }
            }
            else {
                V.TangentX = FVector(1.f, 0.f, 0.f); // Tangent Layer 없으면 기본 X 방향
            }

            // Binormal
            if (binormalLayer)
            {
                int rawIdx = GetRawIndex(binormalLayer, cp, idxCtr, p);
                int finalIdx = GetFinalIndex(binormalLayer, rawIdx);
                if (finalIdx >= 0 && finalIdx < binormalLayer->GetDirectArray().GetCount()) {
                    FbxVector4 b = binormalLayer->GetDirectArray().GetAt(finalIdx);
                    b.Normalize();
                    b = NormalMat.MultT(b);
                    b.Normalize();
                    V.TangentY = FVector((float)b[0], (float)b[1], (float)b[2]); // Z-up 변환
                }
                else {
                    UE_LOG(LogLevel::Warning, TEXT("Invalid final Binormal index %d for vertex (cp=%d, idxCtr=%u, p=%d)"), finalIdx, cp, idxCtr, p);
                    // Binormal은 Normal과 Tangent로부터 계산 가능
                    FVector N = FVector(V.TangentZ.X, V.TangentZ.Y, V.TangentZ.Z);
                    V.TangentY = FVector::CrossProduct(N, V.TangentX);
                }
            }
            else
            {
                // Binormal Layer가 없으면 Normal과 Tangent로부터 계산
                FVector N = FVector(V.TangentZ.X, V.TangentZ.Y, V.TangentZ.Z);
                V.TangentY = FVector::CrossProduct(N, V.TangentX).GetSafeNormal(); // 정규화 추가
            }
            // Tangent 직교화 (선택 사항, 권장 )
            V.TangentX = FVector::CrossProduct(V.TangentY, FVector(V.TangentZ.X, V.TangentZ.Y, V.TangentZ.Z)).GetSafeNormal();


            // Skin influences 
            auto& inf = Influences[cp]; // Control Point 인덱스로 Influences 가져오기
            // 가중치 정렬 및 정규화
            std::sort(inf.begin(), inf.end(), [](auto& A, auto& B) { return A.second > B.second; });
            inf.resize(FMath::Min((int)inf.size(), (int32)MAX_TOTAL_INFLUENCES));
            float totalW = 0.f;
            for (auto& pr : inf) { totalW += (float)pr.second; }

            // 정규화된 가중치 할당 (div by Zero 방지 )
            for (int32 i = 0; i < MAX_TOTAL_INFLUENCES; ++i)
            {
                if (i < inf.size())
                {
                    V.InfluenceBones[i] = (uint8)inf[i].first;
                    V.InfluenceWeights[i] = (totalW > KINDA_SMALL_NUMBER) ? ((float)inf[i].second / totalW) : (i == 0 ? 1.0f : 0.0f); // 0으로 나누기 방지
                }
                else
                {
                    V.InfluenceBones[i] = 0;
                    V.InfluenceWeights[i] = 0.f;
                }
            }
            // 가중치 합계가 1이 되도록 보정 (선택 사항)
            float WeightSum = 0.f;
            for (int32 i = 0; i < MAX_TOTAL_INFLUENCES; ++i) WeightSum += V.InfluenceWeights[i];
            if (WeightSum > KINDA_SMALL_NUMBER) {
                float InvSum = 1.0f / WeightSum;
                for (int32 i = 0; i < MAX_TOTAL_INFLUENCES; ++i) V.InfluenceWeights[i] *= InvSum;
            }


            // --- 최종 데이터 추가 ---
            LodModel.Vertices.Add(V);
            LodModel.Indices.Add(GlobalIdxCtr++); // 수정된: 참조로 받은 GlobalIdxCtr 사용

            idxCtr++; // 폴리곤 내 정점 카운터 증가
        }
    }
    LodModel.NumVertices = LodModel.Vertices.Num();
    LodModel.NumTexCoords = uvCount;
    return true;
}

//------------------------------------------------------------------------------
// FFbxImporter::ParseFbxMaterialTextures
// static helper to parse textures from an FBX material into a FStaticMaterial
//------------------------------------------------------------------------------
bool FFbxImporter::ParseFbxMaterialTextures(FbxSurfaceMaterial* InMaterial, FStaticMaterial& OutMat, const FString& BasePath)
{
    if (!InMaterial || !OutMat.Material)
        return false;

    FObjMaterialInfo& Info = OutMat.Material->GetMaterialInfo();
    Info.TextureInfos.SetNum((int32)EMaterialTextureSlots::MTS_MAX);
    bool bAnyTexture = false;

    auto ParseSlot = [&](const char* PropName, EMaterialTextureSlots Slot, bool bSRGB, EMaterialTextureFlags Flag)
        {
            FbxProperty Prop = InMaterial->FindProperty(PropName);
            if (Prop.IsValid() && Prop.GetSrcObjectCount<FbxFileTexture>() > 0)
            {
                auto* Tex = Prop.GetSrcObject<FbxFileTexture>(0);
                FString FileName = FString(Tex->GetFileName());
                FTextureInfo& TI = Info.TextureInfos[(int32)Slot];
                TI.TextureName = FileName;

                UE_LOG(LogLevel::Warning, TEXT("  Found Texture: Name='%s'"), *FileName);
                FString FullPath;
                // 절대 경로인지 확인 (:' 또는 '/' 로 시작하는지 확인)
                if (FileName.Contains(TEXT(":")) || (!FileName.IsEmpty() && FileName[0] == TEXT('/')))
                {
                    FullPath = FileName; // 절대 경로면 그대로 사용
                    UE_LOG(LogLevel::Warning, TEXT("  Using Absolute Path: '%s'"), *FullPath);
                }
                else
                {
                    // 상대 경로면 BasePath (Contents/FBX/)와 조합
                    FullPath = BasePath + FileName;
                    UE_LOG(LogLevel::Warning, TEXT("  Combining Relative Path: Base='%s', Full='%s'"), *BasePath, *FullPath);
                }

                UE_LOG(LogLevel::Warning, TEXT("  Attempting Path: '%s'"), *FullPath);
                bool bLoaded = FObjLoader::CreateTextureFromFile(FullPath.ToWideString(), bSRGB);
                UE_LOG(LogLevel::Warning, TEXT("  Load Result: %s"), bLoaded ? TEXT("Success") : TEXT("Failed"));
                if (bLoaded)
                {
                    TI.TexturePath = FullPath.ToWideString();
                    TI.bIsSRGB = bSRGB;
                    Info.TextureFlag |= (uint16)Flag;
                    bAnyTexture = true;
                }
                else {
                    UE_LOG(LogLevel::Error, TEXT("  Property invalid or no texture found for %s"), PropName);
                }
            }
        };

    // 다양한 슬롯 파싱
    ParseSlot(FbxSurfaceMaterial::sDiffuse, EMaterialTextureSlots::MTS_Diffuse, true, EMaterialTextureFlags::MTF_Diffuse);
    ParseSlot(FbxSurfaceMaterial::sNormalMap, EMaterialTextureSlots::MTS_Normal, false, EMaterialTextureFlags::MTF_Normal);
    if (!bAnyTexture)
    {
        ParseSlot(FbxSurfaceMaterial::sBump, EMaterialTextureSlots::MTS_Normal, false, EMaterialTextureFlags::MTF_Normal);
        FbxProperty BumpProp = InMaterial->FindProperty(FbxSurfaceMaterial::sBumpFactor);
        if (BumpProp.IsValid())
        {
            double BV = BumpProp.Get<FbxDouble>();
            Info.BumpMultiplier = (float)BV;
        }
    }
    ParseSlot(FbxSurfaceMaterial::sSpecular, EMaterialTextureSlots::MTS_Specular, true, EMaterialTextureFlags::MTF_Specular);
    ParseSlot(FbxSurfaceMaterial::sShininess, EMaterialTextureSlots::MTS_Shininess, false, EMaterialTextureFlags::MTF_Shininess);
    ParseSlot(FbxSurfaceMaterial::sAmbient, EMaterialTextureSlots::MTS_Ambient, true, EMaterialTextureFlags::MTF_Ambient);
    ParseSlot(FbxSurfaceMaterial::sEmissive, EMaterialTextureSlots::MTS_Emissive, true, EMaterialTextureFlags::MTF_Emissive);

    FbxProperty MetProp = InMaterial->FindProperty("Pm");
    if (MetProp.IsValid())
    {
        Info.Metallic = (float)MetProp.Get<FbxDouble>();
        ParseSlot("map_Pm", EMaterialTextureSlots::MTS_Metallic, false, EMaterialTextureFlags::MTF_Metallic);
    }
    FbxProperty RghProp = InMaterial->FindProperty("Pr");
    if (RghProp.IsValid())
    {
        Info.Roughness = (float)RghProp.Get<FbxDouble>();
        ParseSlot("map_Pr", EMaterialTextureSlots::MTS_Roughness, false, EMaterialTextureFlags::MTF_Roughness);
    }

    return bAnyTexture;
}
