#pragma once
#include "Components/MeshComponent.h"
#include "Rendering/SkeletalMeshLODModel.h"

class USkeletalMesh;
class FSkeletalMeshObjectCPUSkin;

class USkinnedMeshComponent : public UMeshComponent
{
    DECLARE_CLASS(USkinnedMeshComponent, UMeshComponent)

public:
    USkinnedMeshComponent();
    void TickComponent(float DeltaTime) override;

    const TArray<FTransform>& GetComponentSpaceTransforms() const;

    FTransform GetBoneLocalTransform(int InBoneIndex) const;
    void SetBoneLocalTransform(int InBoneIndex, const FTransform& InTransform);

    FTransform GetBoneWorldTransform(int InBoneIndex) const;
    void SetBoneWorldTransform(int InBoneIndex, const FTransform& InTransform);
    
    const TArray<FTransform>& GetWorldSpaceTransforms() const;

    virtual USkeletalMesh* GetSkeletalMesh() const { return SkeletalMesh; }
    void SetSkeletalMesh(USkeletalMesh* InSkeletalMesh);

    const TArray<FSoftSkinVertex> GetBindPoseVertices() const { return BindPoseVertices; }
protected:
    FSkeletalMeshObjectCPUSkin* MeshObject;
    /** 매 프레임 변경되는 Runtime Bone Transform - 렌더 스레드 별도일 때의 Size=2 */
    TArray<FTransform> ComponentSpaceTransformsArray; 
    TArray<FTransform> WorldSpaceTransformArray;

    USkeletalMesh* SkeletalMesh = nullptr;

    TArray<FSoftSkinVertex> BindPoseVertices;


protected:
    /** 렌더 스레드에서 읽는 인덱스: 0 Default / 1 Render Thread */
    int32 CurrentReadComponentTransforms; 

    /*virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;*/


};

