#pragma once
#include "Container/Array.h"
#include "Math/Matrix.h"

class USkinnedMeshComponent;

struct FSkeletalMeshRenderData;

struct FSoftSkinVertex;

struct FTransform;

struct FSkeletalMeshVertex;

class FSkeletalMeshObjectCPUSkin
{
public:
    void InitResources(USkinnedMeshComponent* InMeshComponent, FSkeletalMeshRenderData* InSkelMeshRenderData);
    void Update(USkinnedMeshComponent* InMeshComponent, float DeltaTime);
    void SkinVertex(const FSoftSkinVertex& InVertex, TArray<FMatrix> InverseBindPose, TArray<FTransform> BoneGlobalTransforms, FSkeletalMeshVertex& OutVertex);
public:
    USkinnedMeshComponent* MeshComponent;
    FSkeletalMeshRenderData* SkelMeshRenderData;
};

