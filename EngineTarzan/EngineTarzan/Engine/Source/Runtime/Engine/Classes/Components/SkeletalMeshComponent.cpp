#include "SkeletalMeshComponent.h"

#include "SkeletalRenderCPUSkin.h"
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
        if(Bones.Num()>0)
            RotateBone(Bones[BoneIndex], DeltaTime * 100);
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
    //ComponentSpaceTransformsArray[Bone.MyIndex].Translate(FVector(0, 0, -angle));
    ComponentSpaceTransformsArray[Bone.MyIndex].RotatePitch(angle);
    //FRotator CurRot = ComponentSpaceTransformsArray[Bone.MyIndex].GetRotation();
    //ComponentSpaceTransformsArray[Bone.MyIndex].SetRotation(FRotator(CurRot.Pitch, CurRot.Yaw + angle, CurRot.Roll));
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
            WorldSpaceTransformArray[i] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[i];
        }
    }

    //const int32 ParentIndex = Bones[Bone.MyIndex].ParentIndex;
    //if (ParentIndex == -1)
    //{
    //    WorldSpaceTransformArray[Bone.MyIndex] = ComponentSpaceTransformsArray[Bone.MyIndex];
    //}
    //else
    //{
    //    WorldSpaceTransformArray[Bone.MyIndex] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[Bone.MyIndex];
    //}
    //UpdateChildBoneGlobalTransform(Bone.MyIndex);
}

void USkeletalMeshComponent::UpdateChildBoneGlobalTransform(int32 ParentIndex)
{
    const TArray<FMeshBoneInfo>& Bones = SkeletalMesh->GetRefSkeleton().GetBoneInfo();
    const int32 NumBones = Bones.Num();
    for (int32 i = 0; i < NumBones; ++i)
    {
        if (Bones[i].ParentIndex == ParentIndex)
        {
            // 자식 본의 로컬 트랜스폼 → 월드 트랜스폼 갱신
            WorldSpaceTransformArray[i] = WorldSpaceTransformArray[ParentIndex] * ComponentSpaceTransformsArray[i];

            // 재귀적으로 자식의 자식도 갱신
            UpdateChildBoneGlobalTransform(i);
        }
    }
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

    FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetRenderData();

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

