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
    if (SkeletalMesh)
    {
        TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
        if(Bones.Num()>0)
            RotateBone(Bones[BoneIndex], DeltaTime * 100);
    }

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
    ComponentSpaceTransformsArray[Bone.MyIndex].RotatePitch(angle);
    const TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 NumBones = SkeletalMesh->GetRefSkeleton().GetNumBones();
    for (int32 i = 0; i < NumBones; ++i)
    {
        const int32 ParentIndex = Bones[i].ParentIndex;
        const FTransform& Local = ComponentSpaceTransformsArray[i];
        if (ParentIndex == -1)
        {
            WorldSpaceTransformArray[i] = Local;
        }
        else
        {
            WorldSpaceTransformArray[i] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[i];
        }
    }
}

void USkeletalMeshComponent::TranslateBone(const int InBoneIndex, const FVector& InTranslation)
{
    ComponentSpaceTransformsArray[InBoneIndex].Translate(InTranslation);
    const TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 NumBones = SkeletalMesh->GetRefSkeleton().GetNumBones();
    for (int32 i = 0; i < NumBones; ++i)
    {
        const int32 ParentIndex = Bones[i].ParentIndex;
        const FTransform& Local = ComponentSpaceTransformsArray[i];
        if (ParentIndex == -1)
        {
            WorldSpaceTransformArray[i] = Local;
        }
        else
        {
            WorldSpaceTransformArray[i] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[i];
        }
    }
}

void USkeletalMeshComponent::RotateBone(const int InBoneIndex, const FRotator& InRotation)
{
    ComponentSpaceTransformsArray[InBoneIndex].Rotate(InRotation);
    const TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 NumBones = SkeletalMesh->GetRefSkeleton().GetNumBones();
    for (int32 i = 0; i < NumBones; ++i)
    {
        const int32 ParentIndex = Bones[i].ParentIndex;
        const FTransform& Local = ComponentSpaceTransformsArray[i];
        if (ParentIndex == -1)
        {
            WorldSpaceTransformArray[i] = Local;
        }
        else
        {
            WorldSpaceTransformArray[i] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[i];
        }
    }
}

void USkeletalMeshComponent::ScaleBone(const int InBoneIndex, const FVector& InScale)
{
    ComponentSpaceTransformsArray[InBoneIndex].SetScale(InScale);
    const TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 NumBones = SkeletalMesh->GetRefSkeleton().GetNumBones();
    for (int32 i = 0; i < NumBones; ++i)
    {
        const int32 ParentIndex = Bones[i].ParentIndex;
        const FTransform& Local = ComponentSpaceTransformsArray[i];
        if (ParentIndex == -1)
        {
            WorldSpaceTransformArray[i] = Local;
        }
        else
        {
            WorldSpaceTransformArray[i] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[i];
        }
    }
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
