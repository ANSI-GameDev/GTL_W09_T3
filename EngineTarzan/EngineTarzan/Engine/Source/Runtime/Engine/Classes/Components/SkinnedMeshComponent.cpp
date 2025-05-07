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
    FFbxImporter::ParseSkeletalMeshLODModel(
        //TEXT("Contents/FBX/Spider.fbx"),
        TEXT("Contents/FBX/nathan3.fbx"),
        //TEXT("Contents/FBX/Mir4/source/Mon_BlackDragon31_Skeleton.fbx"),
        //TEXT("Contents/FBX/tifa2.fbx"),
        //TEXT("Contents/FBX/tifa_noglove/tifanoglove.fbx"),
        //TEXT("Contents/FBX/aerith.fbx"),
        //TEXT("Contents/FBX/tifamaterial/PC0002_00_BodyB.fbx"),
        *SkeletalMesh->ImportedModel,
        &SkeletalMesh->RefSkeleton
    );

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
