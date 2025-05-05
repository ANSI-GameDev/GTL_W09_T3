#include "FbxImporter.h"
//#include <fbxsdk.h>

#include "Define.h"
#include "ReferenceSkeleton.h"
#include "Rendering/SkeletalMeshLODModel.h"

TMap<FbxNode*, int32> FFbxImporter::NodeToBoneIndex;
FbxAMatrix            FFbxImporter::JointPostConvert;
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

    // 6) 축 & 단위 보정
    ComputeJointPostConvert(Scene);

    // 7) SDK 정리
    Scene->Destroy();
    SdkMgr->Destroy();

    // 8) 결과 로깅
    bool bSuccess = (OutRefSkeleton.GetNumBones() > 0);
    UE_LOG(LogLevel::Error, TEXT("ReferenceSkeleton parsing %s"), bSuccess ? TEXT("succeeded") : TEXT("failed"));
    return bSuccess;
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
        FTransform BonePose(
            FVector(Local.GetT()[0], Local.GetT()[1], Local.GetT()[2]),
            FQuat(FRotator(Local.GetR()[0], Local.GetR()[1], Local.GetR()[2])),
            FVector(Local.GetS()[0], Local.GetS()[1], Local.GetS()[2])
        );

        // OutRefSkeleton 에 본 추가
        ThisParent = OutRefSkeleton.AddBone(BoneName, ParentIndex, BonePose);
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

// --- 축 & 단위 보정 ---
void FFbxImporter::ComputeJointPostConvert(FbxScene* Scene)
{
    FbxAxisSystem SceneAxis = Scene->GetGlobalSettings().GetAxisSystem();
    FbxAxisSystem UnrealAxis(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityEven, FbxAxisSystem::eLeftHanded);

    FbxAMatrix Mscene, Munreal;
    SceneAxis.GetMatrix(Mscene);
    UnrealAxis.GetMatrix(Munreal);
    FbxAMatrix AxisConv = Munreal * Mscene.Inverse();

    FbxSystemUnit SysUnit = Scene->GetGlobalSettings().GetSystemUnit();
    double Scale = 1.0 / SysUnit.GetScaleFactor();
    FbxAMatrix UnitScale; UnitScale.SetIdentity();
    UnitScale.SetS(FbxVector4(Scale, Scale, Scale));

    JointPostConvert = UnitScale * AxisConv;
}

// --- LODModel 파일 파싱 및 본 계층 적용 ---
bool FFbxImporter::ParseSkeletalMeshLODModel(
    const FString& InFilePath,
    FSkeletalMeshLODModel& LodModel,
    FReferenceSkeleton* OutRefSkeleton
)
{
    // 1) SDK 초기화
    FbxManager* SdkMgr = FbxManager::Create();
    FbxIOSettings* IOS = FbxIOSettings::Create(SdkMgr, IOSROOT);
    SdkMgr->SetIOSettings(IOS);
    FbxImporter* Importer = FbxImporter::Create(SdkMgr, TEXT(""));
    if (!Importer->Initialize(*InFilePath, -1, SdkMgr->GetIOSettings()))
    {
        return false;
    }

    // 2) Scene 생성 및 임포트
    FbxScene* Scene = FbxScene::Create(SdkMgr, TEXT("Scene"));
    Importer->Import(Scene);
    Importer->Destroy();

    // 3) Reference Skeleton 재사용 빌드
    if (OutRefSkeleton)
    {
        BuildReferenceSkeleton(Scene->GetRootNode(), *OutRefSkeleton, INDEX_NONE, 0);
        UE_LOG(LogLevel::Error, TEXT("ReferenceSkeleton built from existing Scene"));
    }

    // 4) 축/단위 보정
    ComputeJointPostConvert(Scene);

    // 5) 첫 번째 메시 노드 찾기
    FbxMesh* FoundMesh = nullptr;
    TArray<FbxNode*> Queue;
    Queue.Add(Scene->GetRootNode());
    for (int32 i = 0; i < Queue.Num() && !FoundMesh; ++i)
    {
        FbxNode* Node = Queue[i];
        if (Node->GetMesh()) { FoundMesh = Node->GetMesh(); break; }
        for (int32 c = 0; c < Node->GetChildCount(); ++c) Queue.Add(Node->GetChild(c));
    }

    bool bResult = false;
    if (FoundMesh)
    {
        bResult = ParseSkeletalMeshLODModel(FoundMesh, LodModel);
    }

    // 6) 정리
    Scene->Destroy();
    SdkMgr->Destroy();
    return bResult;
}


bool FFbxImporter::ParseSkeletalMeshLODModel(FbxMesh* Mesh, FSkeletalMeshLODModel& LodModel)
{
    if (!Mesh) return false;

    // 0) 노말·탄젠트 생성 및 레이어 찾기
    if (Mesh->GetLayerCount() == 0 || !Mesh->GetLayer(0)->GetNormals())
    {
        Mesh->InitNormals(); Mesh->GenerateNormals(true);
    }
    if (!Mesh->GetLayer(0)->GetTangents())
    {
        Mesh->InitTangents(); Mesh->GenerateTangentsData(0, true, true);
    }
    if (!Mesh->GetLayer(0)->GetBinormals())
    {
        Mesh->InitBinormals(); Mesh->CreateElementBinormal();
    }

    // 레이어 검색
    FbxLayerElementNormal* normalLayer = nullptr;
    FbxLayerElementTangent* tangentLayer = nullptr;
    FbxLayerElementBinormal* binormalLayer = nullptr;
    for (int li = 0;li < Mesh->GetLayerCount();++li)
    {
        auto layer = Mesh->GetLayer(li);
        if (!normalLayer && layer->GetNormals())
        {
            normalLayer = layer->GetNormals();
        }
        if (!tangentLayer && layer->GetTangents())
        {
            tangentLayer = layer->GetTangents();
        }
        if (!binormalLayer && layer->GetBinormals())
        {
            binormalLayer = layer->GetBinormals();
        }
    }

    // 1) 변환
    FbxNode* Node = Mesh->GetNode();
    FbxAMatrix Geo; Geo.SetIdentity();
    Geo.SetT(Node->GetGeometricTranslation(FbxNode::eSourcePivot)); Geo.SetR(Node->GetGeometricRotation(FbxNode::eSourcePivot)); Geo.SetS(Node->GetGeometricScaling(FbxNode::eSourcePivot));
    Geo = Geo * JointPostConvert;
    FbxAMatrix NormalMat = Geo.Inverse().Transpose();

    // 2) 스킨 처리
    int32 CPCount = Mesh->GetControlPointsCount();
    std::vector<std::vector<std::pair<int32, double>>> Influences(CPCount);
    TMap<int32, FbxAMatrix> BindPoseMap;
    for (int di = 0;di < Mesh->GetDeformerCount(FbxDeformer::eSkin);++di)
    {
        auto* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(di, FbxDeformer::eSkin));
        for (int ci = 0;ci < Skin->GetClusterCount();++ci)
        {
            FbxCluster* Cluster = Skin->GetCluster(ci);
            FbxAMatrix Mlink, Minitial; Cluster->GetTransformLinkMatrix(Mlink); Cluster->GetTransformMatrix(Minitial);
            FbxAMatrix Bind = Minitial.Inverse() * Mlink * JointPostConvert;
            FbxNode* LinkNode = Cluster->GetLink();
            int32* FoundIdx = NodeToBoneIndex.Find(LinkNode);
            if (!FoundIdx) { UE_LOG(LogLevel::Warning, TEXT("Unmapped bone: %s"), *FString(LinkNode->GetName())); continue; }
            int32 BoneIdx = *FoundIdx;
            if (!BindPoseMap.Contains(BoneIdx)) { BindPoseMap.Add(BoneIdx, Bind.Inverse()); LodModel.RequiredBones.Add(BoneIdx); }
            UE_LOG(LogLevel::Error, TEXT("Cluster %d -> Bone '%s' idx=%d"), ci, *FString(LinkNode->GetName()), BoneIdx);
            int cnt = Cluster->GetControlPointIndicesCount(); int* cpis = Cluster->GetControlPointIndices(); double* wts = Cluster->GetControlPointWeights();
            for (int k = 0;k < cnt;++k) Influences[cpis[k]].emplace_back(BoneIdx, wts[k]);
        }
    }

    // 3) RefBasesInvMatrix
    LodModel.RefBasesInvMatrix.Empty();
    for (int32 Bi : LodModel.RequiredBones)
    {
        LodModel.RefBasesInvMatrix.Add(ConvertFbxAMatrix(BindPoseMap[Bi]));
    }

    // 4) 폴리곤->삼각형 & 정점 채우기
    int32 uvCount = FMath::Min(Mesh->GetElementUVCount(), (int32)MAX_TEXCOORDS);

    LodModel.Vertices.Empty();
    LodModel.Indices.Empty();
    LodModel.Faces.Empty();

    uint32 idxCtr = 0;
    int32 polyCount = Mesh->GetPolygonCount();
    for (int32 p = 0;p < polyCount;++p)
    {
        int32 triCnt = Mesh->GetPolygonSize(p) - 2;
        for (int t = 0;t < triCnt;++t)
        {
            LodModel.Faces.Add(p);
        }
        for (int t = 0;t < triCnt;++t) 
        {
            int vtxIdx[3] = { 0,t + 2,t + 1 };
            for (int f = 0;f < 3;++f) 
            {
                int32 cp = Mesh->GetPolygonVertex(p, vtxIdx[f]); FSoftSkinVertex V{};
                auto P = Geo.MultT(Mesh->GetControlPoints()[cp]); V.Position = FVector((float)P[0], (float)P[2], (float)P[1]);

                // Normal
                if (normalLayer)
                {
                    int rawIdx = (normalLayer->GetMappingMode() == FbxLayerElement::eByControlPoint)
                        ? cp
                        : idxCtr;                                        // ← 매핑 모드 처리
                    int finalIdx;
                    if (normalLayer->GetReferenceMode() == FbxLayerElement::eDirect)
                        finalIdx = rawIdx;
                    else  // eIndexToDirect
                        finalIdx = normalLayer->GetIndexArray().GetAt(rawIdx); // ← 참조 모드 처리

                    FbxVector4 n = normalLayer->GetDirectArray().GetAt(finalIdx);
                    n = NormalMat.MultT(n);
                    V.TangentZ = FVector4((float)n[0], (float)n[2], (float)n[1], 0);
                }

                // Tangent
                if (tangentLayer)
                {
                    int rawIdx = (tangentLayer->GetMappingMode() == FbxLayerElement::eByControlPoint)
                        ? cp
                        : idxCtr;                                        // ← 매핑 모드 처리
                    int finalIdx;
                    if (tangentLayer->GetReferenceMode() == FbxLayerElement::eDirect)
                        finalIdx = rawIdx;
                    else
                        finalIdx = tangentLayer->GetIndexArray().GetAt(rawIdx); // ← 참조 모드 처리

                    FbxVector4 t = tangentLayer->GetDirectArray().GetAt(finalIdx);
                    t = NormalMat.MultT(t);
                    V.TangentX = FVector((float)t[0], (float)t[2], (float)t[1]);
                }

                // Binormal
                if (binormalLayer)
                {
                    int rawIdx = (binormalLayer->GetMappingMode() == FbxLayerElement::eByControlPoint)
                        ? cp
                        : idxCtr;                                        // ← 매핑 모드 처리
                    int finalIdx;
                    if (binormalLayer->GetReferenceMode() == FbxLayerElement::eDirect)
                        finalIdx = rawIdx;
                    else
                        finalIdx = binormalLayer->GetIndexArray().GetAt(rawIdx); // ← 참조 모드 처리

                    FbxVector4 b = binormalLayer->GetDirectArray().GetAt(finalIdx);
                    b = NormalMat.MultT(b);
                    V.TangentY = FVector((float)b[0], (float)b[2], (float)b[1]);
                }

                // Skin influences
                auto& inf = Influences[cp];
                std::sort(inf.begin(), inf.end(), [](auto& A, auto& B) {return A.second > B.second;});
                inf.resize(FMath::Min((int)inf.size(), (int32)MAX_TOTAL_INFLUENCES));
                float totalWeight = 0;

                for (auto& pr : inf)
                {
                    totalWeight += (float)pr.second;
                }
                for (int32 i = 0;i < MAX_TOTAL_INFLUENCES;++i)
                {
                    if (i < inf.size())
                    {
                        V.InfluenceBones[i] = (uint8)inf[i].first;
                        V.InfluenceWeights[i] = inf[i].second / totalWeight;
                    }
                    else
                    {
                        V.InfluenceBones[i] = 0;V.InfluenceWeights[i] = 0;
                    }
                }
                LodModel.Vertices.Add(V); LodModel.Indices.Add(idxCtr++);
            }
        }
    }
    LodModel.NumVertices = LodModel.Vertices.Num(); LodModel.NumTexCoords = uvCount;
    return true;
}
