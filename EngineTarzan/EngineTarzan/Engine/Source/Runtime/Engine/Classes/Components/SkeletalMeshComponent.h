#pragma once
#include "SkinnedMeshComponent.h"

class USkeletalMesh;

struct FMeshBoneInfo;

class USkeletalMeshComponent :public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)
public:
    USkeletalMeshComponent();
    void TickComponent(float DeltaTime) override;
    
    void ResetBoneTransform();

    void RotateBone(FMeshBoneInfo Bone, float angle);
    
    void TranslateBone(int InBoneIndex, const FVector& InTranslation);
    
    void RotateBone(int InBoneIndex, const FRotator& InRotation);

    void ScaleBone(int InBoneIndex, const FVector& InScale);

    

    int BoneIndex = 0;

    void SetSelectedSubMeshIndex(const int& value) { SelectedSubMeshIndex = value; }
    int GetSelectedSubMeshIndex() const { return SelectedSubMeshIndex; }
    //virtual UObject* Duplicate(UObject* InOuter) override;
    //virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    //virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
private:
    int SelectedSubMeshIndex = -1;
};

