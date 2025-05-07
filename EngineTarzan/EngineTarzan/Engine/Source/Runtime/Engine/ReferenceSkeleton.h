#pragma once
#include "Container/Array.h"
#include "Container/Map.h"
#include "Math/FTransform.h"
#include "UObject/NameTypes.h"
#include <Engine/Engine.h>
#include <Actors/Cube.h>
#include "World/World.h"

struct FMeshBoneInfo
{
    FName Name;                // Bone 이름
    int32 MyIndex;             // 내 인덱스
    int32 ParentIndex;         // 부모 인덱스
    FTransform LocalTransform; // 부모 기준 로컬 트랜스폼

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
        int32 MyIndex = RefBoneInfo.Num();
        RefBoneInfo.Add({ InBoneName, MyIndex, InParentIndex, InBonePose });
        //RefBonePose.Add(InBonePose);
        BoneNameToIndexMap.Add(InBoneName, RefBoneInfo.Num() - 1);

        return MyIndex;
    }

    void ComputeGlobalTransform()
    {
        const uint32 BoneNum = RefBoneInfo.Num();
        RefBonePose.SetNum(BoneNum);
        for (uint32 i = 0; i < BoneNum; ++i)
        {
            const uint32 ParentIndex = RefBoneInfo[i].ParentIndex;
            const FTransform& Local = RefBoneInfo[i].LocalTransform;
            if (ParentIndex == -1)
                RefBonePose[i] = Local;
            else
            {
                RefBonePose[i] = RefBonePose[ParentIndex] * Local;
            }
			//ACube* BoneActor = GEngine->ActiveWorld->SpawnActor<ACube>();
			//BoneActor->SetActorLocation(RefBonePose[i].GetPosition() / 100.0f);
			//BoneActor->SetActorRotation(RefBonePose[i].GetRotation());
			//BoneActor->SetActorScale(FVector(0.01f));
        }
    }

    void InitializeInverseBindPose()
    {
        for (const auto& BindPose : RefBonePose)
        {
            InverseBindPose.Add(FMatrix::Inverse(BindPose.GetMatrix()));
        }
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
    const TArray<FMatrix>& GetInverseBindPose() const { return InverseBindPose; }

public:
    TArray<FMeshBoneInfo> RefBoneInfo; 
    TArray<FTransform>    RefBonePose; //각 본의 글로벌 트랜스폼
    TArray<FMatrix>       InverseBindPose;
    TMap<FName, int32> BoneNameToIndexMap;

    bool bOnlyOneRootAllowed;
};
