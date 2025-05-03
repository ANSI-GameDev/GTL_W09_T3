#include "FbxImporter.h"
//#include <fbxsdk.h>

#include "Define.h"
#include "ReferenceSkeleton.h"
#include "Rendering/SkeletalMeshLODModel.h"

TMap<FbxNode*, int32> FFbxImporter::NodeToBoneIndex;
FbxAMatrix            FFbxImporter::JointPostConvert;

bool FFbxImporter::ParseReferenceSkeleton(const FString& InFilePath, FReferenceSkeleton& OutRefSkeleton)
{
    // FBX SDK 초기화
    FbxManager* SdkMgr = FbxManager::Create();
    FbxIOSettings* IOSettings = FbxIOSettings::Create(SdkMgr, IOSROOT);
    SdkMgr->SetIOSettings(IOSettings);

    // Importer
    FbxImporter* Importer = FbxImporter::Create(SdkMgr, "");
    if (!Importer->Initialize(*InFilePath, -1, SdkMgr->GetIOSettings()))
    {
        UE_LOG(LogLevel::Error,TEXT("Failed to initialize FBX Importer for %s"), *InFilePath);
        Importer->Destroy();
        SdkMgr->Destroy();
        return false;
    }

    // Scene 생성 및 임포트
    FbxScene* Scene = FbxScene::Create(SdkMgr, "Scene");
    Importer->Import(Scene);
    Importer->Destroy();

    // Root 노드부터 재귀 빌드
    BuildReferenceSkeleton(Scene->GetRootNode(), OutRefSkeleton, INDEX_NONE, 0);

    ComputeJointPostConvert(Scene);

    // SDK 정리
    SdkMgr->Destroy();

    bool bSuccess = OutRefSkeleton.GetNumBones() > 0;
    UE_LOG(LogLevel::Error, TEXT("ReferenceSkeleton parsing %s"), bSuccess ? TEXT("succeeded") : TEXT("failed"));
    return bSuccess;
}

USkeletalMesh* FFbxImporter::ImportSkeletalMesh(const FString& InFilePath)
{

    // @TODO: 메시, 스킨 웨이트, 애니메이션 등 임포트 구현
    UE_LOG(LogLevel::Warning, TEXT("ImportSkeletalMesh not yet implemented for %s"), *InFilePath);
    return nullptr;

}


void FFbxImporter::BuildReferenceSkeleton(FbxNode* Node, FReferenceSkeleton& OutRefSkeleton, uint32 ParentIndex, int32 Depth)
{
    if (!Node) return;

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

        ThisParent = OutRefSkeleton.AddBone(BoneName, ParentIndex, BonePose);

        NodeToBoneIndex.Add(Node, ThisParent);

        // 트리 구조 형태 출력
        FString Prefix;
        if (Depth > 0)
        {
            Prefix = TEXT("  "); // 첫 들여쓰기
            for (int32 i = 1; i < Depth; ++i)
            {
                Prefix += TEXT("  "); 
            }
            Prefix += TEXT("|__"); 
        }
        // 루트 노드는 Prefix없이
        UE_LOG(LogLevel::Error, TEXT("%s Bone: %s (ParentIndex: %d, MyIndex: %d)"), *Prefix, *BoneName.ToString(), ParentIndex, ThisParent);
    }

    // 자식 노드 순회
    for (int32 i = 0; i < Node->GetChildCount(); ++i)
    {
        BuildReferenceSkeleton(Node->GetChild(i), OutRefSkeleton, ThisParent, Depth + 1);
    }
}

FMatrix FFbxImporter::ConvertFbxAMatrix(const FbxAMatrix& M)
{
    // double[16] → float[16] 순서 그대로 복사
    FMatrix Out;
    const double* Src = reinterpret_cast<const double*>(&M);
    float* Dst = reinterpret_cast<float*>(&Out);
    for (int i = 0; i < 16; ++i) Dst[i] = (float)Src[i];
    return Out;
}

void FFbxImporter::ComputeJointPostConvert(FbxScene* Scene)
{
    // 1) FBX 씬 축계
    FbxAxisSystem SceneAxis = Scene->GetGlobalSettings().GetAxisSystem();
    // 2)  2) 언리얼 축계: Z-Up, ParityOdd(front=+X), Left-Handed
    FbxAxisSystem UnrealAxis(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityEven, FbxAxisSystem::eLeftHanded);

    // 3) 축 변환 행렬: SceneAxis → UnrealAxis
    FbxAMatrix Mscene, Munreal;
    SceneAxis.GetMatrix(Mscene);
    UnrealAxis.GetMatrix(Munreal);
    FbxAMatrix AxisConv = Munreal * Mscene.Inverse();

    // 4) 단위 변환: FBX 단위 → 언리얼(cm)
    FbxSystemUnit SysUnit = Scene->GetGlobalSettings().GetSystemUnit();
    double Scale = 1.0 / SysUnit.GetScaleFactor();
    FbxAMatrix UnitScale; UnitScale.SetIdentity();
    UnitScale.SetS(FbxVector4(Scale, Scale, Scale));

    // 최종 보정 매트릭스
    JointPostConvert = UnitScale * AxisConv;
}

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
    FbxImporter* Importer = FbxImporter::Create(SdkMgr, "");
    if (!Importer->Initialize(*InFilePath, -1, SdkMgr->GetIOSettings()))
        return false;

    // 2) 씬 생성 및 임포트
    FbxScene* Scene = FbxScene::Create(SdkMgr, "Scene");
    Importer->Import(Scene);
    Importer->Destroy();

    // 3) (선택) 본 계층 파싱
    if (OutRefSkeleton)
    {
        ParseReferenceSkeleton(InFilePath, *OutRefSkeleton);
    }

    // 4) 축/단위 보정 매트릭스 계산
    ComputeJointPostConvert(Scene);

    // 5) 씬 트리를 순회하여 첫 FbxMesh 파싱
    FbxMesh* FoundMesh = nullptr;
    TArray<FbxNode*> Queue;
    Queue.Add(Scene->GetRootNode());
    for (int32 i = 0; i < Queue.Num() && !FoundMesh; ++i)
    {
        FbxNode* Node = Queue[i];
        if (Node->GetMesh())
        {
            FoundMesh = Node->GetMesh();
            break;
        }
        for (int32 c = 0; c < Node->GetChildCount(); ++c)
            Queue.Add(Node->GetChild(c));
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

    // --- 0) Normal/Tangent/Binormal 확보 ---
    int layerCount = Mesh->GetLayerCount();
    if (layerCount == 0 || Mesh->GetLayer(0)->GetNormals() == nullptr)
    {
        Mesh->InitNormals();
        Mesh->GenerateNormals(true);
    }
    if (layerCount == 0
        || Mesh->GetLayer(0)->GetTangents() == nullptr
        || Mesh->GetLayer(0)->GetBinormals() == nullptr)
    {
        Mesh->InitTangents();
        Mesh->InitBinormals();
        Mesh->CreateElementBinormal();
        Mesh->GenerateTangentsData(0, true, false);
    }

    // --- 1) Geo 트랜스폼 & NormalMat ---
    FbxNode* Node = Mesh->GetNode();
    FbxAMatrix Geo; Geo.SetIdentity();
    Geo.SetT(Node->GetGeometricTranslation(FbxNode::eSourcePivot));
    Geo.SetR(Node->GetGeometricRotation(FbxNode::eSourcePivot));
    Geo.SetS(Node->GetGeometricScaling(FbxNode::eSourcePivot));
    Geo = Geo * JointPostConvert;
    FbxAMatrix NormalMat = Geo.Inverse().Transpose();

    // --- 2) 스킨 클러스터 처리 ---
    int32 CPCount = Mesh->GetControlPointsCount();
    std::vector<std::vector<std::pair<int32, double>>> Influences(CPCount);
    TMap<int32, FbxAMatrix> BindPoseMap;
    for (int di = 0; di < Mesh->GetDeformerCount(FbxDeformer::eSkin); ++di)
    {
        auto* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(di, FbxDeformer::eSkin));
        for (int ci = 0; ci < Skin->GetClusterCount(); ++ci)
        {
            FbxCluster* Cluster = Skin->GetCluster(ci);
            FbxAMatrix Mlink = Cluster->GetTransformLinkMatrix(Mlink);
            FbxAMatrix Minitial = Cluster->GetTransformMatrix(Minitial);
            FbxAMatrix Bind = Minitial.Inverse() * Mlink * JointPostConvert;

            int32 BoneIdx = NodeToBoneIndex[Cluster->GetLink()];
            if (!BindPoseMap.Contains(BoneIdx))
            {
                BindPoseMap.Add(BoneIdx, Bind.Inverse());
                LodModel.RequiredBones.Add(BoneIdx);
            }

            int cnt = Cluster->GetControlPointIndicesCount();
            int* cpis = Cluster->GetControlPointIndices();
            double* wts = Cluster->GetControlPointWeights();
            for (int k = 0; k < cnt; ++k)
                Influences[cpis[k]].emplace_back(BoneIdx, wts[k]);
        }
    }

    // --- 3) RefBasesInvMatrix 채우기 ---
    LodModel.RefBasesInvMatrix.Empty();
    for (int32 Bi : LodModel.RequiredBones)
    {
        LodModel.RefBasesInvMatrix.Add(ConvertFbxAMatrix(BindPoseMap[Bi]));
    }

    // --- 4) Layer 요소 포인터 ---
    auto* layer0 = Mesh->GetLayer(0);
    auto* normalLayer = layer0->GetNormals();
    auto* tangentLayer = layer0->GetTangents();
    auto* binormalLayer = layer0->GetBinormals();
    auto* colorLayer = layer0->GetVertexColors();
    int32 uvCount = FMath::Min(Mesh->GetElementUVCount(), (int32)MAX_TEXCOORDS);
    TArray<FbxGeometryElementUV*> uvLayers;
    for (int i = 0; i < uvCount; ++i)
        uvLayers.Add(Mesh->GetElementUV(i));

    // --- 5) 폴리곤→트라이앵글 분해 & 정점 채우기 ---
    LodModel.Vertices.Empty();
    LodModel.Indices.Empty();
    LodModel.Faces.Empty();

    int32 polyCount = Mesh->GetPolygonCount();
    uint32 idxCtr = 0;
    for (int32 p = 0; p < polyCount; ++p)
    {
        int32 polySize = Mesh->GetPolygonSize(p);
        int32 triCnt = polySize - 2;

        // Faces: 폴리곤 인덱스
        for (int t = 0; t < triCnt; ++t)
            LodModel.Faces.Add(p);

        // Fan 분해: 삼각형 하나당 세 꼭짓점
        for (int t = 0; t < triCnt; ++t)
        {
            int vtxIdx[3] = { 0, t + 2, t + 1 };
            for (int f = 0; f < 3; ++f)
            {
                int32 cpIdx = Mesh->GetPolygonVertex(p, vtxIdx[f]);
                FSoftSkinVertex V{};

                // Position
                auto P = Geo.MultT(Mesh->GetControlPoints()[cpIdx]);
                V.Position = FVector((float)P[0], (float)P[2], (float)P[1]);

                // UVs
                for (int u = 0; u < uvCount; ++u)
                {
                    int idx = Mesh->GetTextureUVIndex(p, vtxIdx[f]);
                    auto uv = uvLayers[u]->GetDirectArray().GetAt(idx);
                    V.UVs[u] = FVector2D((float)uv[0], 1.f - (float)uv[1]);
                }

                // Color
                if (colorLayer)
                {
                    int idx = Mesh->GetTextureUVIndex(p, vtxIdx[f]);
                    auto c = colorLayer->GetDirectArray().GetAt(idx);
                    V.Color = FColor((uint8)(c.mRed * 255), (uint8)(c.mGreen * 255), (uint8)(c.mBlue * 255));
                }

                // Normal
                if (normalLayer)
                {
                    FbxVector4 N = (normalLayer->GetMappingMode() == FbxGeometryElement::eByControlPoint)
                        ? normalLayer->GetDirectArray().GetAt(cpIdx)
                        : normalLayer->GetDirectArray().GetAt(idxCtr);
                    N = NormalMat.MultT(N);
                    V.TangentZ = FVector4((float)N[0], (float)N[2], (float)N[1], 0);
                }

                // Tangent / Binormal
                if (tangentLayer)
                {
                    FbxVector4 T = (tangentLayer->GetMappingMode() == FbxGeometryElement::eByControlPoint)
                        ? tangentLayer->GetDirectArray().GetAt(cpIdx)
                        : tangentLayer->GetDirectArray().GetAt(idxCtr);
                    T = NormalMat.MultT(T);
                    V.TangentX = FVector((float)T[0], (float)T[2], (float)T[1]);
                }
                if (binormalLayer)
                {
                    FbxVector4 B = (binormalLayer->GetMappingMode() == FbxGeometryElement::eByControlPoint)
                        ? binormalLayer->GetDirectArray().GetAt(cpIdx)
                        : binormalLayer->GetDirectArray().GetAt(idxCtr);
                    B = NormalMat.MultT(B);
                    V.TangentY = FVector((float)B[0], (float)B[2], (float)B[1]);
                }

                // Skin influences
                auto& inf = Influences[cpIdx];
                std::sort(inf.begin(), inf.end(),
                    [](auto& A, auto& B) { return A.second > B.second; });
                inf.resize(FMath::Min((int)inf.size(), (int32)MAX_TOTAL_INFLUENCES));
                float totW = 0.f; for (auto& pr : inf) totW += (float)pr.second;
                for (int32 i = 0; i < MAX_TOTAL_INFLUENCES; ++i)
                {
                    if (i < inf.size())
                    {
                        V.InfluenceBones[i] = (uint8)inf[i].first;
                        V.InfluenceWeights[i] = (float)(inf[i].second / totW);
                    }
                    else
                    {
                        V.InfluenceBones[i] = 0;
                        V.InfluenceWeights[i] = 0.f;
                    }
                }

                LodModel.Vertices.Add(V);
                LodModel.Indices.Add(idxCtr++);
            }
        }
    }

    LodModel.NumVertices = LodModel.Vertices.Num();
    LodModel.NumTexCoords = uvCount;
    return true;
}
