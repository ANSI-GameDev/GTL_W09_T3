#include "SkeletalRenderCPUSkin.h"

#include "Container/Array.h"

#include "Rendering/SkeletalMeshLODModel.h"


void FSkeletalMeshObjectCPUSkin::InitResources(USkinnedMeshComponent* InMeshComponent, FSkeletalMeshRenderData* InSkelMeshRenderData)
{
    MeshComponent = InMeshComponent;
    SkelMeshRenderData = InSkelMeshRenderData;
}

void FSkeletalMeshObjectCPUSkin::Update(USkinnedMeshComponent* InMeshComponent, float DeltaTime)
{
    // @TODO : CPU Skinning 로직을 여기에 구현
    // 1. SkeletalMeshRenderData에서 Vertices, Indices 가져오기
    // 2. ComponentSpaceTransformsArray에서 Bone Transform 가져오기
    // 3. Vertex를 Bone Transform에 맞게 변환
    // 4. 변환된 Vertex를 Weight를 적절히 적용하여 Skinning을 구현할 것

    //FSkeletalMeshLODModel*
}
