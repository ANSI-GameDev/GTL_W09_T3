#pragma once
#include "Container/Array.h"

class USkinnedMeshComponent;

struct FSkeletalMeshRenderData;

struct FSoftSkinVertex;

struct FTransform;

class FSkeletalMeshObjectCPUSkin
{
public:
    void InitResources(USkinnedMeshComponent* InMeshComponent, FSkeletalMeshRenderData* InSkelMeshRenderData);
    void Update(USkinnedMeshComponent* InMeshComponent, float DeltaTime);
    void SkinVertex(FSoftSkinVertex& Vertex, TArray<FTransform> BindPoseTransforms, TArray<FTransform> InverseBase);

public:
    USkinnedMeshComponent* MeshComponent;
    FSkeletalMeshRenderData* SkelMeshRenderData;
};

