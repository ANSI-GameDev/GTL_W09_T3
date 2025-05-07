#include "SkinnedMeshComponent.h"

#include "SkeletalRenderCPUSkin.h"
#include "Developer/SkeletalMeshBuilder.h"
#include "Engine/SkeletalMesh.h"
#include "UnrealEd/FbxImporter.h"


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
        MeshObject->Update(this, DeltaTime);
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
}

FTransform USkinnedMeshComponent::GetBoneWorldTransform(const int InBoneIndex) const
{
    return WorldSpaceTransformArray[InBoneIndex];
}

void USkinnedMeshComponent::SetBoneWorldTransform(const int InBoneIndex, const FTransform& InTransform)
{
    WorldSpaceTransformArray[InBoneIndex] = InTransform;
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
}
