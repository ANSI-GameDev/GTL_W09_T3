#pragma once
#include "Container/Array.h"
#include "Math/Matrix.h"

class USkinnedMeshComponent;

struct FSkeletalMeshRenderData;

struct FSoftSkinVertex;

struct FTransform;

class FSkeletalMeshObjectCPUSkin
{
public:
    void InitResources(USkinnedMeshComponent* InMeshComponent, FSkeletalMeshRenderData* InSkelMeshRenderData);
    void Update(USkinnedMeshComponent* InMeshComponent, float DeltaTime);
    void SkinVertex(FSoftSkinVertex& Vertex, TArray<FMatrix> InverseBindPose, TArray<FTransform> GlobalTransforms);

public:
    USkinnedMeshComponent* MeshComponent;
    FSkeletalMeshRenderData* SkelMeshRenderData;
};

