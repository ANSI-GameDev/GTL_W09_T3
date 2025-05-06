#include "SkeletalActor.h"
#include "Components/SkeletalMeshComponent.h"
#include <Components/StaticMeshComponent.h>
#include <Engine/FObjLoader.h>
#include "Engine/SkeletalMesh.h"

ASkeletalActor::ASkeletalActor()
{
    UE_LOG(LogLevel::Error, TEXT("SkeletalActor::ASkeletalActor()"));
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>();
    RootComponent = SkeletalMeshComponent;
    SetActorTickInEditor(true);
    int BoneNum = SkeletalMeshComponent->GetSkeletalMesh()->GetRefSkeleton().GetNumBones();
    for (int i = 0; i < BoneNum; ++i)
    {
        UStaticMeshComponent* Bone = AddComponent<UStaticMeshComponent>();
        Bone->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/helloBlender.obj"));
        Bone->SetWorldLocation(SkeletalMeshComponent->GetSkeletalMesh()->GetRefSkeleton().GetBonePose()[i].GetPosition());
        Bone->SetWorldRotation(SkeletalMeshComponent->GetSkeletalMesh()->GetRefSkeleton().GetBonePose()[i].GetRotation());
        Bone->SetWorldScale3D(FVector(0.01f));
        Bones.Add(Bone);
    }
}

USkeletalMeshComponent* ASkeletalActor::GetSkeletalMeshComponent() const
{
    return SkeletalMeshComponent;
}

void ASkeletalActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TArray<FTransform> BoneWorldTransforms = SkeletalMeshComponent->GetWorldSpaceTransforms();
    int BoneNum = Bones.Num();
    for (int i = 0; i < BoneNum; ++i)
    {
        Bones[i]->SetWorldLocation(BoneWorldTransforms[i].GetPosition());
        Bones[i]->SetWorldRotation(BoneWorldTransforms[i].GetRotation());
        if (i == SkeletalMeshComponent->BoneIndex)
        {
            Bones[i]->SetWorldScale3D(FVector(0.02f));
        }
        else
        {
            Bones[i]->SetWorldScale3D(FVector(0.01f));
        }
    }
}
