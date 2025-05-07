#include "SkeletalMeshComponent.h"

#include "SkeletalRenderCPUSkin.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Asset/SkeletalMeshAsset.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "ReferenceSkeleton.h"

USkeletalMeshComponent::USkeletalMeshComponent()
{
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
