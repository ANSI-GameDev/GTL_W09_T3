#pragma once


class USkinnedMeshComponent;

struct FSkeletalMeshRenderData;

class FSkeletalMeshObjectCPUSkin
{
public:
    void InitResources(USkinnedMeshComponent* InMeshComponent, FSkeletalMeshRenderData* InSkelMeshRenderData);
    void Update(USkinnedMeshComponent* InMeshComponent, float DeltaTime);

public:
    USkinnedMeshComponent* MeshComponent;
    FSkeletalMeshRenderData* SkelMeshRenderData;
};

