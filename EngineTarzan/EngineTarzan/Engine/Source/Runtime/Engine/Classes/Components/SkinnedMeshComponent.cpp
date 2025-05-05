#include "SkinnedMeshComponent.h"

#include "SkeletalRenderCPUSkin.h"
#include "Engine/SkeletalMesh.h"


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
