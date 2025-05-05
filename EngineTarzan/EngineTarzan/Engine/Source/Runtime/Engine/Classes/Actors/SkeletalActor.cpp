#include "SkeletalActor.h"
#include "Components/SkeletalMeshComponent.h"

ASkeletalActor::ASkeletalActor()
{
    UE_LOG(LogLevel::Error, TEXT("SkeletalActor::ASkeletalActor()"));
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>();
    RootComponent = SkeletalMeshComponent;
}

USkeletalMeshComponent* ASkeletalActor::GetSkeletalMeshComponent() const
{
    return SkeletalMeshComponent;
}
