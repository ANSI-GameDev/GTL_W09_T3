#pragma once
#include "Components/MeshComponent.h"

class USkeletalMesh;
class FSkeletalMeshObjectCPUSkin;

class USkinnedMeshComponent : public UMeshComponent
{
    DECLARE_CLASS(USkinnedMeshComponent, UMeshComponent)

public:
    USkinnedMeshComponent();
    void TickComponent(float DeltaTime) override;

    const TArray<FTransform>& GetComponentSpaceTransforms() const;

    USkeletalMesh* GetSkeletalMesh() const { return SkeletalMesh; }

protected:
    FSkeletalMeshObjectCPUSkin* MeshObject;
    /** 매 프레임 변경되는 Runtime Bone Transform - 렌더 스레드 별도일 때의 Size=2 */
    TArray<FTransform> ComponentSpaceTransformsArray; 

    USkeletalMesh* SkeletalMesh = nullptr;


protected:
    /** 렌더 스레드에서 읽는 인덱스: 0 Default / 1 Render Thread */
    int32 CurrentReadComponentTransforms; 

    /*virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;*/


};

