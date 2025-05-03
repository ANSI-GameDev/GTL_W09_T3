#pragma once
#include "SkinnedMeshComponent.h"

class USkeletalMeshCompnent :public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshCompnent, USkinnedMeshComponent)
public:
    USkeletalMeshCompnent() = default;
    //virtual UObject* Duplicate(UObject* InOuter) override;
    //virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    //virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

};

