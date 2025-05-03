#include "SkinnedMeshComponent.h"


const TArray<FTransform>& USkinnedMeshComponent::GetComponentSpaceTransforms() const
{
    return ComponentSpaceTransformsArray;
}
