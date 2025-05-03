#pragma once
#include "Components/MeshComponent.h"

class USkeletalMesh;

class USkinnedMeshComponent : public UMeshComponent
{
    DECLARE_CLASS(USkinnedMeshComponent, UMeshComponent)

public:
    USkinnedMeshComponent() = default;

    const TArray<FTransform>& GetComponentSpaceTransforms() const;

protected:
    /** 매 프레임 변경되는 Runtime Bone Transform - 렌더 스레드 별도일 때의 Size=2 */
    TArray<FTransform> ComponentSpaceTransformsArray[2]; 

    USkeletalMesh* SkeletalMesh = nullptr;


protected:
    /** 렌더 스레드에서 읽는 인덱스: 0 Default / 1 Render Thread */
    int32 CurrentReadComponentTransforms; 

    /*virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;*/


};

