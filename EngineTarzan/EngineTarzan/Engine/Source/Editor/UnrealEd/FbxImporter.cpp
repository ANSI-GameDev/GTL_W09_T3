#include "FbxImporter.h"
//#include <fbxsdk.h>

#include "Define.h"
#include "ReferenceSkeleton.h"

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

        // 트리 구조 형태 출력
        FString Prefix;
        if (Depth > 0)
        {
            Prefix = TEXT("  "); // 첫 들여쓰기
            for (int32 i = 1; i < Depth; ++i)
            {
                Prefix += TEXT("|  "); 
            }
            Prefix += TEXT("=="); 
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
