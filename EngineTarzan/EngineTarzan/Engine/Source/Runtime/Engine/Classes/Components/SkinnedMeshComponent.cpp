#include "SkinnedMeshComponent.h"

#include "SkeletalRenderCPUSkin.h"
#include "Developer/SkeletalMeshBuilder.h"
#include "Engine/SkeletalMesh.h"
#include "UnrealEd/FbxImporter.h"
#include "Engine/Asset/SkeletalMeshAsset.h"

USkinnedMeshComponent::USkinnedMeshComponent()
    : MeshObject(nullptr)
{
    //MeshObject = new FSkeletalMeshObjectCPUSkin();
    //MeshObject->InitResources(this, SkeletalMesh->SkelMeshRenderData);
}

void USkinnedMeshComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (MeshObject)
    {
        MeshObject->Update(this);
    }

    // Update ComponentSpaceTransforms();
    // Update RenderData Resources
}

const TArray<FTransform>& USkinnedMeshComponent::GetComponentSpaceTransforms() const
{
    return ComponentSpaceTransformsArray;
}

FTransform USkinnedMeshComponent::GetBoneLocalTransform(const int InBoneIndex) const
{
    return ComponentSpaceTransformsArray[InBoneIndex];
}

void USkinnedMeshComponent::SetBoneLocalTransform(const int InBoneIndex, const FTransform& InTransform)
{
    ComponentSpaceTransformsArray[InBoneIndex] = InTransform;
    const FMeshBoneInfo& Bone = SkeletalMesh->GetRefSkeleton().GetBoneInfo()[InBoneIndex];
    if (Bone.ParentIndex == -1)
    {
        WorldSpaceTransformArray[InBoneIndex] = ComponentSpaceTransformsArray[InBoneIndex];
    }
    else
    {
        WorldSpaceTransformArray[InBoneIndex] = WorldSpaceTransformArray[Bone.ParentIndex] * ComponentSpaceTransformsArray[InBoneIndex];
    }
    UpdateChildBoneGlobalTransform(InBoneIndex);

    MeshObject->Update(this);
}

FTransform USkinnedMeshComponent::GetBoneWorldTransform(const int InBoneIndex) const
{
    return WorldSpaceTransformArray[InBoneIndex];
}

void USkinnedMeshComponent::SetBoneWorldTransform(const int InBoneIndex, const FTransform& InTransform)
{
    WorldSpaceTransformArray[InBoneIndex] = InTransform;
    UpdateChildBoneGlobalTransform(InBoneIndex);

    MeshObject->Update(this);
}

const TArray<FTransform>& USkinnedMeshComponent::GetWorldSpaceTransforms() const
{
    return WorldSpaceTransformArray;
}

void USkinnedMeshComponent::SetSkeletalMesh(USkeletalMesh* InSkeletalMesh)
{
    SkeletalMesh = InSkeletalMesh;
    
    MeshObject = new FSkeletalMeshObjectCPUSkin;
    MeshObject->InitResources(this, SkeletalMesh->GetRenderData());

    //FFbxImporter::ParseReferenceSkeleton("Contents/FBX/Anime_character.fbx", SkeletalMesh->RefSkeleton);
    //Contents/FBX/Mir4/source/Mon_BlackDragon31_Skeleton.FBX

    BindPoseVertices.Empty();
    for (const auto& Vertex : SkeletalMesh->ImportedModel->Vertices)
    {
        BindPoseVertices.Add(Vertex);
    }

    WorldSpaceTransformArray.Empty();
    for (auto& Transform : SkeletalMesh->RefSkeleton.GetBonePose())
    {
        WorldSpaceTransformArray.Add(Transform);
    }

    ComponentSpaceTransformsArray.Empty();
    for (auto& Bone : SkeletalMesh->RefSkeleton.GetBoneInfo())
    {
        ComponentSpaceTransformsArray.Add(Bone.LocalTransform);
    }

    FSkeletalMeshBuilder::BuildRenderData(
        *SkeletalMesh->ImportedModel,
        *SkeletalMesh->SkelMeshRenderData
    );

    AABB.MinLocation = SkeletalMesh->SkelMeshRenderData->BoundingBoxMin;
    AABB.MaxLocation = SkeletalMesh->SkelMeshRenderData->BoundingBoxMax;
}

void USkinnedMeshComponent::UpdateChildBoneGlobalTransform(int32 ParentIndex)
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
