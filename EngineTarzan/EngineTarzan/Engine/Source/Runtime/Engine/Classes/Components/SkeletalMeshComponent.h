#pragma once
#include "SkinnedMeshComponent.h"

struct FMeshBoneInfo;

class USkeletalMeshComponent :public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)
public:
    USkeletalMeshComponent();
    void TickComponent(float DeltaTime) override;

    USkeletalMesh* GetSkeletalMesh() const { return SkeletalMesh; }

    void ResetBoneTransform();

    void RotateBonePitch(FMeshBoneInfo Bone, float angle);

    int BoneIndex = 0;

    //virtual UObject* Duplicate(UObject* InOuter) override;
    //virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    //virtual void SetProperties(const TMap<FString, FString>& InProperties) override;


};

