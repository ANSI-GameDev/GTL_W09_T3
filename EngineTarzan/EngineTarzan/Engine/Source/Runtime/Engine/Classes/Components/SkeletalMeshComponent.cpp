#include "SkeletalMeshComponent.h"

#include <fstream>

#include "SkeletalRenderCPUSkin.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "UnrealEd/FbxImporter.h"
#include "UObject/ObjectFactory.h"

USkeletalMeshComponent::USkeletalMeshComponent()
{
    UE_LOG(LogLevel::Error, TEXT("SkeletalMeshComponent::USkeletalMeshComponent()"));
    SkeletalMesh= FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);
    SkeletalMesh->Initialize(); // ImportedModel과 SkelMeshRenderData 생성

    //FFbxImporter::ParseReferenceSkeleton("Contents/FBX/Anime_character.fbx", SkeletalMesh->RefSkeleton);

    FFbxImporter::ParseSkeletalMeshLODModel(
        TEXT("Contents/FBX/Anime_character.fbx"),
        *SkeletalMesh->ImportedModel,
        &SkeletalMesh->RefSkeleton
    );
    // --- 디버깅용 파일 출력 ---
    std::ofstream ofs("FBXDebug.txt", std::ios::out | std::ios::trunc);
    if (!ofs.is_open())
    {
        // 파일 열기 실패
        return;
    }

    // 1) ReferenceSkeleton 덤프
    ofs << "=== ReferenceSkeleton ===\n";
    const auto& BoneInfos = SkeletalMesh->RefSkeleton.GetBoneInfo();
    const auto& BonePoses = SkeletalMesh->RefSkeleton.GetBonePose();
    for (int32 i = 0; i < BoneInfos.Num(); ++i)
    {
        // BoneInfo
        ofs << "[" << i << "] Name="
            << std::string(*BoneInfos[i].Name.ToString())
            << " Parent=" << BoneInfos[i].ParentIndex << "\n";

        // BonePose (FTransform → location, rotation, scale)
        const FTransform& T = BonePoses[i];
        auto Loc = T.GetPosition();
        auto RotQ = T.GetRotation();
        auto Scale = T.GetScale();
        ofs << "    Loc=("
            << Loc.X << "," << Loc.Y << "," << Loc.Z << ") "
            << "RotQ=("
            << RotQ.Yaw << "," << RotQ.Pitch << "," << RotQ.Roll << ") "
            << "Scale=("
            << Scale.X << "," << Scale.Y << "," << Scale.Z << ")\n";
    }
    ofs << "\n";

    // 2) LODModel 덤프
    ofs << "=== LODModel ===\n";
    FSkeletalMeshLODModel& L = *SkeletalMesh->ImportedModel;
    ofs << "NumVertices=" << L.NumVertices
        << " NumTexCoords=" << L.NumTexCoords << "\n";

    ofs << "RequiredBones: ";
    for (int32 b : L.RequiredBones)
        ofs << b << " ";
    ofs << "\n\n";

    ofs << "RefBasesInvMatrix:\n";
    for (int32 i = 0; i < L.RefBasesInvMatrix.Num(); ++i)
    {
        const FMatrix& M = L.RefBasesInvMatrix[i];
        ofs << "[" << i << "] ";
        for (int row = 0; row < 4; ++row)
        {
            ofs << "[";
            for (int col = 0; col < 4; ++col)
            {
                ofs << M.M[row][col];
                if (col < 3) ofs << ", ";
            }
            ofs << "] ";
        }
        ofs << "\n";
    }
    ofs << "\n";

    ofs << "Vertices:\n";
    for (uint32 i = 0; i < L.Vertices.Num(); ++i)
    {
        const auto& V = L.Vertices[i];
        ofs << "[" << i << "] Pos=("
            << V.Position.X << "," << V.Position.Y << "," << V.Position.Z << ") "
            << "TanX=("
            << V.TangentX.X << "," << V.TangentX.Y << "," << V.TangentX.Z << ") "
            << "TanY=("
            << V.TangentY.X << "," << V.TangentY.Y << "," << V.TangentY.Z << ") "
            << "Nor=("
            << V.TangentZ.X << "," << V.TangentZ.Y << "," << V.TangentZ.Z << ")\n";
    }
    ofs << "\n";

    ofs << "Indices:\n";
    for (uint32 idx : L.Indices)
        ofs << idx << " ";
    ofs << "\n\n";

    ofs << "Faces:\n";
    for (uint32 f : L.Faces)
        ofs << f << " ";
    ofs << "\n";

    ofs.close();

    // 5) 렌더 리소스 초기화
    // MeshObject = new FSkeletalMeshObjectCPUSkin();
    // MeshObject->InitResources(this, SkeletalMesh->SkelMeshRenderData);

}

void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
