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
}

USkeletalMeshComponent* ASkeletalActor::GetSkeletalMeshComponent() const
{
    return SkeletalMeshComponent;
}

void ASkeletalActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
