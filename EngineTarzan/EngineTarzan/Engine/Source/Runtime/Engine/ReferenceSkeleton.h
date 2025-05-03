#pragma once
#include "Container/Array.h"
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

private:
    TArray<FMeshBoneInfo> RefBonInfo;
    TArray<FTransform>    RefBonePose;


    bool bOnlyOneRootAllowed;
};
