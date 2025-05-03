#pragma once
#include "SkinnedMeshComponent.h"

class USkeletalMeshComponent :public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)
public:
    USkeletalMeshComponent() = default;
    //virtual UObject* Duplicate(UObject* InOuter) override;
    //virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    //virtual void SetProperties(const TMap<FString, FString>& InProperties) override;


};

