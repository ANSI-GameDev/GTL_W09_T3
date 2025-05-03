#pragma once
#include "Container/Array.h"
#include "Container/Map.h"
#include "Math/FTransform.h"
#include "UObject/NameTypes.h"


struct FMeshBoneInfo
{
    FName Name;             // Bone 이름
    int32 ParentIndex;      // 부모 인덱스

    bool operator==(const FMeshBoneInfo& B) const
    {
        return(Name == B.Name);
    }

    friend FArchive& operator<<(FArchive& Ar, FMeshBoneInfo& F);
};
struct FReferenceSkeleton
{
    FReferenceSkeleton(bool bInOnlyOneRootAllowed = true)
        :bOnlyOneRootAllowed(bInOnlyOneRootAllowed)
    {
    }

public:
    /** Bone 정보를 추가하고 인덱스를 반환 */
    uint32 AddBone(FName InBoneName, int32 InParentIndex, const FTransform& InBonePose)
    {
        RefBoneInfo.Add({ InBoneName, InParentIndex });
        RefBonePose.Add(InBonePose);
        BoneNameToIndexMap.Add(InBoneName, RefBoneInfo.Num() - 1);

        return RefBoneInfo.Num() - 1;
    }

    /** Name으로 Bone을 찾고, 인덱스를 반환 */
    int32 FindRawBoneIndex(FName BoneName) const
    {
        if (const int32* Index = BoneNameToIndexMap.Find(BoneName))
        {
            return *Index;
        }
        return INDEX_NONE;
    }

    int32 GetNumBones() const { return RefBoneInfo.Num(); }
    const TArray<FMeshBoneInfo>& GetBoneInfo() const { return RefBoneInfo; }
    const TArray<FTransform>& GetBonePose() const { return RefBonePose; }

private:
    TArray<FMeshBoneInfo> RefBoneInfo;
    TArray<FTransform>    RefBonePose;

    TMap<FName, int32> BoneNameToIndexMap;

    bool bOnlyOneRootAllowed;
};
