#include "SkeletalMeshComponent.h"

#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Asset/SkeletalMeshAsset.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "ReferenceSkeleton.h"

USkeletalMeshComponent::USkeletalMeshComponent()
{
}

void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
    if (SkeletalMesh)
    {
        TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
        //if(Bones.Num()>0)
        //    RotateBone(Bones[BoneIndex], DeltaTime * 100);
    }

    Super::TickComponent(DeltaTime);
}

void USkeletalMeshComponent::ResetBoneTransform()
{
    ComponentSpaceTransformsArray.Empty();
    WorldSpaceTransformArray.Empty();
    for (auto& Transform : SkeletalMesh->RefSkeleton.GetBonePose())
    {
        WorldSpaceTransformArray.Add(Transform);
    }
    for (auto& Bone : SkeletalMesh->RefSkeleton.GetBoneInfo())
    {
        ComponentSpaceTransformsArray.Add(Bone.LocalTransform);
    }
    SkeletalMesh->ImportedModel->Vertices.Empty();
    for (const auto& Vertex : BindPoseVertices)
    {
        SkeletalMesh->ImportedModel->Vertices.Add(Vertex);
    }
}

void USkeletalMeshComponent::RotateBone(FMeshBoneInfo Bone, float angle)
{
    ComponentSpaceTransformsArray[Bone.MyIndex].RotatePitch(angle);
    const TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 NumBones = SkeletalMesh->GetRefSkeleton().GetNumBones();
    for (int32 i = 0; i < NumBones; ++i)
    {
        const int32 ParentIndex = Bones[i].ParentIndex;
        const FTransform& Local = ComponentSpaceTransformsArray[i];
        if (ParentIndex == -1)
        {
            WorldSpaceTransformArray[i] = Local;
        }
        else
        {
            WorldSpaceTransformArray[i] = WorldSpaceTransformArray[ParentIndex] * Local;
        }
    }
}

void USkeletalMeshComponent::TranslateBone(const int InBoneIndex, const FVector& InTranslation)
{
    ComponentSpaceTransformsArray[InBoneIndex].Translate(InTranslation);
    const TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 ParentIndex = Bones[InBoneIndex].ParentIndex;
    if (ParentIndex == -1)
    {
        WorldSpaceTransformArray[InBoneIndex] = ComponentSpaceTransformsArray[InBoneIndex];
    }
    else
    {
        WorldSpaceTransformArray[InBoneIndex] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[InBoneIndex];
    }
    UpdateChildBoneGlobalTransform(InBoneIndex);
}

void USkeletalMeshComponent::RotateBone(const int InBoneIndex, const FRotator& InRotation)
{
    ComponentSpaceTransformsArray[InBoneIndex].Rotate(InRotation);
    const TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 ParentIndex = Bones[InBoneIndex].ParentIndex;
    if (ParentIndex == -1)
    {
        WorldSpaceTransformArray[InBoneIndex] = ComponentSpaceTransformsArray[InBoneIndex];
    }
    else
    {
        WorldSpaceTransformArray[InBoneIndex] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[InBoneIndex];
    }
    UpdateChildBoneGlobalTransform(InBoneIndex);
}

void USkeletalMeshComponent::ScaleBone(const int InBoneIndex, const FVector& InScale)
{
    FVector OriginalScale = ComponentSpaceTransformsArray[InBoneIndex].GetScale();
    ComponentSpaceTransformsArray[InBoneIndex].SetScale(OriginalScale + InScale);
    const TArray<FMeshBoneInfo> Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 ParentIndex = Bones[InBoneIndex].ParentIndex;
    if (ParentIndex == -1)
    {
        WorldSpaceTransformArray[InBoneIndex] = ComponentSpaceTransformsArray[InBoneIndex];
    }
    else
    {
        WorldSpaceTransformArray[InBoneIndex] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[InBoneIndex];
    }
    UpdateChildBoneGlobalTransform(InBoneIndex);
}


int USkeletalMeshComponent::CheckRayIntersection(const FVector& InRayOrigin, const FVector& InRayDirection, float& OutHitDistance) const
{
    if (!AABB.Intersect(InRayOrigin, InRayDirection, OutHitDistance))
    {
        return 0;
    }
    if (SkeletalMesh == nullptr)
    {
        return 0;
    }
    OutHitDistance = FLT_MAX;

    int IntersectionNum = 0;

    std::shared_ptr<FSkeletalMeshRenderData> RenderData = SkeletalMesh->GetSkeletalMeshRenderData();

    const TArray<FSkeletalMeshVertex>& Vertices = RenderData->Vertices;
    const int32 VertexNum = Vertices.Num();
    if (VertexNum == 0)
    {
        return 0;
    }

    const TArray<UINT>& Indices = RenderData->Indices;
    const int32 IndexNum = Indices.Num();
    const bool bHasIndices = (IndexNum > 0);

    int32 TriangleNum = bHasIndices ? (IndexNum / 3) : (VertexNum / 3);
    for (int32 i = 0; i < TriangleNum; i++)
    {
        int32 Idx0 = i * 3;
        int32 Idx1 = i * 3 + 1;
        int32 Idx2 = i * 3 + 2;

        if (bHasIndices)
        {
            Idx0 = Indices[Idx0];
            Idx1 = Indices[Idx1];
            Idx2 = Indices[Idx2];
        }

        FVector v0 = FVector(Vertices[Idx0].X, Vertices[Idx0].Y, Vertices[Idx0].Z);
        FVector v1 = FVector(Vertices[Idx1].X, Vertices[Idx1].Y, Vertices[Idx1].Z);
        FVector v2 = FVector(Vertices[Idx2].X, Vertices[Idx2].Y, Vertices[Idx2].Z);

        float HitDistance = FLT_MAX;
        if (IntersectRayTriangle(InRayOrigin, InRayDirection, v0, v1, v2, HitDistance))
        {
            OutHitDistance = FMath::Min(HitDistance, OutHitDistance);
            IntersectionNum++;
        }

    }
    return IntersectionNum;
}

