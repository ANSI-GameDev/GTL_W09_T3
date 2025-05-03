#pragma once
#include "Components/MeshComponent.h"

class USkeletalMesh;

class USkinnedMeshComponent : public UMeshComponent
{
    DECLARE_CLASS(USkinnedMeshComponent, UMeshComponent)

public:
    USkinnedMeshComponent() = default;

protected:
    USkeletalMesh* SkeletalMesh = nullptr;

    /*virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;*/


};

