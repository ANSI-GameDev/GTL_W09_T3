#include "SkeletalMeshComponent.h"

#include <fstream>

#include "SkeletalRenderCPUSkin.h"
#include "Developer/SkeletalMeshBuilder.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Asset/SkeletalMeshAsset.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "UnrealEd/FbxImporter.h"
#include "UObject/ObjectFactory.h"
#include "ReferenceSkeleton.h"

USkeletalMeshComponent::USkeletalMeshComponent()
{
    UE_LOG(LogLevel::Error, TEXT("SkeletalMeshComponent::USkeletalMeshComponent()"));
    // SkeletalMesh= FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);
    // SkeletalMesh->Initialize(); // ImportedModel과 SkelMeshRenderData 생성
    //
    // MeshObject = new FSkeletalMeshObjectCPUSkin;
    // MeshObject->InitResources(this, SkeletalMesh->GetRenderData());
    //
    // //FFbxImporter::ParseReferenceSkeleton("Contents/FBX/Anime_character.fbx", SkeletalMesh->RefSkeleton);
    // //Contents/FBX/Mir4/source/Mon_BlackDragon31_Skeleton.FBX
    // FFbxImporter::ParseSkeletalMeshLODModel(
    //     //TEXT("Contents/FBX/Spider.fbx"),
    //     TEXT("Contents/FBX/nathan3.fbx"),
    //     //TEXT("Contents/FBX/Mir4/source/Mon_BlackDragon31_Skeleton.fbx"),
    //     //TEXT("Contents/FBX/tifa2.fbx"),
    //     //TEXT("Contents/FBX/tifa_noglove/tifanoglove.fbx"),
    //     //TEXT("Contents/FBX/aerith.fbx"),
    //     //TEXT("Contents/FBX/tifamaterial/PC0002_00_BodyB.fbx"),
    //     *SkeletalMesh->ImportedModel,
    //     &SkeletalMesh->RefSkeleton
    // );

    // for (const auto& Vertex : SkeletalMesh->ImportedModel->Vertices)
    // {
    //     BindPoseVertices.Add(Vertex);
    // }
    //
    // WorldSpaceTransformArray.Empty();
    // for (auto& Transform : SkeletalMesh->RefSkeleton.GetBonePose())
    // {
    //     WorldSpaceTransformArray.Add(Transform);
    // }
    //
    // ComponentSpaceTransformsArray.Empty();
    // for (auto& Bone : SkeletalMesh->RefSkeleton.GetBoneInfo())
    // {
    //     ComponentSpaceTransformsArray.Add(Bone.LocalTransform);
    // }
    //
    // FSkeletalMeshBuilder::BuildRenderData(
    //     *SkeletalMesh->ImportedModel,
    //     *SkeletalMesh->SkelMeshRenderData
    // );

    // // --- 디버깅용 파일 출력 ---
    // std::ofstream ofs("FBXDebug.txt", std::ios::out | std::ios::trunc);
    // if (!ofs.is_open())
    // {
    //     // 파일 열기 실패
    //     return;
    // }
    //
    // // 1) ReferenceSkeleton 덤프
    // ofs << "=== ReferenceSkeleton ===\n";
    // const auto& BoneInfos = SkeletalMesh->RefSkeleton.GetBoneInfo();
    // const auto& BonePoses = SkeletalMesh->RefSkeleton.GetBonePose();
    // for (int32 i = 0; i < BoneInfos.Num(); ++i)
    // {
    //     // BoneInfo
    //     ofs << "[" << i << "] Name="
    //         << std::string(*BoneInfos[i].Name.ToString())
    //         << " Parent=" << BoneInfos[i].ParentIndex << "\n";
    //
    //     // BonePose (FTransform → location, rotation, scale)
    //     const FTransform& T = BonePoses[i];
    //     auto Loc = T.GetPosition();
    //     auto RotQ = T.GetRotation();
    //     auto Scale = T.GetScale();
    //     ofs << "    Loc=("
    //         << Loc.X << "," << Loc.Y << "," << Loc.Z << ") "
    //         << "RotQ=("
    //         << RotQ.Yaw << "," << RotQ.Pitch << "," << RotQ.Roll << ") "
    //         << "Scale=("
    //         << Scale.X << "," << Scale.Y << "," << Scale.Z << ")\n";
    // }
    // ofs << "\n";
    //
    // // 2) LODModel 덤프
    // ofs << "=== LODModel ===\n";
    // FSkeletalMeshLODModel& L = *SkeletalMesh->ImportedModel;
    // ofs << "NumVertices=" << L.NumVertices
    //     << " NumTexCoords=" << L.NumTexCoords << "\n";
    //
    // // RequiredBones
    // ofs << "RequiredBones: ";
    // for (int32 b : L.RequiredBones)
    //     ofs << b << " ";
    // ofs << "\n\n";
    //
    // // RefBasesInvMatrix
    // ofs << "RefBasesInvMatrix:\n";
    // for (int32 i = 0; i < L.RefBasesInvMatrix.Num(); ++i)
    // {
    //     const FMatrix& M = L.RefBasesInvMatrix[i];
    //     ofs << "[" << i << "] ";
    //     for (int row = 0; row < 4; ++row)
    //     {
    //         ofs << "[";
    //         for (int col = 0; col < 4; ++col)
    //         {
    //             ofs << M.M[row][col];
    //             if (col < 3) ofs << ", ";
    //         }
    //         ofs << "] ";
    //     }
    //     ofs << "\n";
    // }
    // ofs << "\n";
    //
    // // TODO : 일단 주석 처리 나중에 해제
    // // 3) Sections 덤프  ← 추가
    //  ofs << "=== Sections ===\n";
    //  for (int32 i = 0; i < L.Sections.Num(); ++i)
    //  {
    //      const auto& S = L.Sections[i];
    //      ofs << "[" << i << "] MaterialIndex=" << S.MaterialIndex
    //          << " BaseIndex=" << S.BaseIndex
    //          << " NumTriangles=" << S.NumTriangles
    //          << " BaseVertexIndex=" << S.BaseVertexIndex << "\n";
    //      ofs << "    BoneMap: ";
    //      for (int32 bi : S.BoneMap)
    //          ofs << bi << " ";
    //      ofs << "\n\n";
    //  }
    //
    // //4) Vertices 덤프 (UV, Color, Influence까지) ← 확장
    // ofs << "=== Vertices ===\n";
    // for (uint32 i = 0; i < L.Vertices.Num(); ++i)
    // {
    //     const auto& V = L.Vertices[i];
    //     ofs << "[" << i << "] Pos=("
    //         << V.Position.X << "," << V.Position.Y << "," << V.Position.Z << ")";
    //
    //     // UVs
    //     for (uint32 u = 0; u < L.NumTexCoords; ++u)
    //     {
    //         ofs << " UV" << u << "=("
    //             << V.UVs[u].X << "," << V.UVs[u].Y << ")";
    //     }
    //
    //     // Vertex Color
    //     ofs << " Color=("
    //         << (int)V.Color.R << "," << (int)V.Color.G << ","
    //         << (int)V.Color.B << "," << (int)V.Color.A << ")";
    //
    //     // Tangents & Normal
    //     ofs << " TanX=("
    //         << V.TangentX.X << "," << V.TangentX.Y << "," << V.TangentX.Z << ")"
    //         << " TanY=("
    //         << V.TangentY.X << "," << V.TangentY.Y << "," << V.TangentY.Z << ")"
    //         << " Nor=("
    //         << V.TangentZ.X << "," << V.TangentZ.Y << "," << V.TangentZ.Z << ")";
    //
    //     // Skin Influences
    //     ofs << " Influences=[";
    //     for (int inf = 0; inf < MAX_TOTAL_INFLUENCES; ++inf)
    //     {
    //         ofs << "("
    //             << (int)V.InfluenceBones[inf] << ","
    //             << V.InfluenceWeights[inf] << ")";
    //         if (inf + 1 < MAX_TOTAL_INFLUENCES) ofs << ",";
    //     }
    //     ofs << "]\n";
    // }
    // ofs << "\n";
    //
    // // 5) Indices 덤프
    // ofs << "=== Indices ===\n";
    // for (uint32 idx : L.Indices)
    //     ofs << idx << " ";
    // ofs << "\n\n";
    //
    // // 6) Faces 덤프
    // ofs << "=== Faces ===\n";
    // for (uint32 f : L.Faces)
    //     ofs << f << " ";
    // ofs << "\n";
    //
    // // 7) Materials 덤프
    // ofs << "=== Materials ===\n";
    // for (int32 mi = 0; mi < SkeletalMesh->GetRenderData()->Materials.Num(); ++mi)
    // {
    //     const FObjMaterialInfo& MI = SkeletalMesh->GetRenderData()->Materials[mi];
    //     ofs << "[" << mi << "] Name=" << std::string(*MI.MaterialName) << "\n";
    //     ofs << "  Diffuse=(" << MI.DiffuseColor.X << "," << MI.DiffuseColor.Y << "," << MI.DiffuseColor.Z << ")\n";
    //     ofs << "  Specular=(" << MI.SpecularColor.X << "," << MI.SpecularColor.Y << "," << MI.SpecularColor.Z << ")\n";
    //     ofs << "  Ambient=(" << MI.AmbientColor.X << "," << MI.AmbientColor.Y << "," << MI.AmbientColor.Z << ")\n";
    //     ofs << "  Emissive=(" << MI.EmissiveColor.X << "," << MI.EmissiveColor.Y << "," << MI.EmissiveColor.Z << ")\n";
    //     ofs << "  Metallic=" << MI.Metallic << " Roughness=" << MI.Roughness << "\n";
    //     ofs << "  TextureFlag=0x" << std::hex << MI.TextureFlag << std::dec << "\n";
    //     for (int32 ti = 0; ti < MI.TextureInfos.Num(); ++ti)
    //     {
    //         const FTextureInfo& TI = MI.TextureInfos[ti];
    //         ofs << "    [" << ti << "] Name=" << *FString(TI.TextureName)
    //             << " Path=" << *FString(TI.TexturePath)
    //             << " sRGB=" << (TI.bIsSRGB ? "true" : "false") << "\n";
    //     }
    //     ofs << "\n";
    // }
    // ofs.close();

    // 5) 렌더 리소스 초기화
    // MeshObject = new FSkeletalMeshObjectCPUSkin();
    // MeshObject->InitResources(this, SkeletalMesh->SkelMeshRenderData);

}

void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
    TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    RotateBone(Bones[BoneIndex], DeltaTime);
    Super::TickComponent(DeltaTime);
}

void USkeletalMeshComponent::ResetBoneTransform()
{
    ComponentSpaceTransformsArray.Empty();
    WorldSpaceTransformArray.Empty();
    for (auto& Transform : SkeletalMesh->RefSkeleton.GetBonePose())
    {
        WorldSpaceTransformArray.Add(Transform);
    }
    for (auto& Bone : SkeletalMesh->RefSkeleton.GetBoneInfo())
    {
        ComponentSpaceTransformsArray.Add(Bone.LocalTransform);
    }
    SkeletalMesh->ImportedModel->Vertices.Empty();
    for (const auto& Vertex : BindPoseVertices)
    {
        SkeletalMesh->ImportedModel->Vertices.Add(Vertex);
    }
}

void USkeletalMeshComponent::RotateBone(FMeshBoneInfo Bone, float angle)
{
    //ComponentSpaceTransformsArray[Bone.MyIndex].RotatePitch(angle);
    FRotator CurRot = ComponentSpaceTransformsArray[Bone.MyIndex].GetRotation();
    ComponentSpaceTransformsArray[Bone.MyIndex].SetRotation(FRotator(CurRot.Pitch, CurRot.Yaw + angle, CurRot.Roll));
    const TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();

    const int32 ParentIndex = Bones[Bone.MyIndex].ParentIndex;
    if (ParentIndex == -1)
    {
        WorldSpaceTransformArray[Bone.MyIndex] = ComponentSpaceTransformsArray[Bone.MyIndex];
    }
    else
    {
        WorldSpaceTransformArray[Bone.MyIndex] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[Bone.MyIndex];
    }
    UpdateChildBoneGlobalTransform(Bone.MyIndex);
}

void USkeletalMeshComponent::UpdateChildBoneGlobalTransform(int32 ParentIndex)
{
    const TArray<FMeshBoneInfo>& Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 NumBones = Bones.Num();
    for (int32 i = 0; i < NumBones; ++i)
    {
        if (Bones[i].ParentIndex == ParentIndex)
        {
            // 자식 본의 로컬 트랜스폼 → 월드 트랜스폼 갱신
            WorldSpaceTransformArray[i] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[i];

            // 재귀적으로 자식의 자식도 갱신
            UpdateChildBoneGlobalTransform(i);
        }
    }
}
