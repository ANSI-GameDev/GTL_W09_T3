#pragma once
#include "Container/Array.h"
#include "Math/Matrix.h"

struct FDualQuat;
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

    void SkinVertexOptimized(const FSoftSkinVertex& Src, const TArray<FMatrix>& SkinnedMatrices, FSkeletalMeshVertex& Out);

    void SkinVertexDualQuat(
        const FSoftSkinVertex& Src,
        const TArray<FDualQuat>& SkinDQs,
        FSkeletalMeshVertex& Out);
public:
    USkinnedMeshComponent* MeshComponent;
    FSkeletalMeshRenderData* SkelMeshRenderData;
};

