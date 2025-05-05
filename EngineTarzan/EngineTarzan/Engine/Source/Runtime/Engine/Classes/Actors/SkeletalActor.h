#pragma once
#include "GameFramework/Actor.h"
#include "UObject/ObjectMacros.h"

class USkeletalMeshComponent;

class ASkeletalActor : public AActor
{
    DECLARE_CLASS(ASkeletalActor, AActor)
public:
    ASkeletalActor();
    USkeletalMeshComponent* GetSkeletalMeshComponent() const;

protected:
    UPROPERTY
    (USkeletalMeshComponent*, SkeletalMeshComponent, = nullptr);
};

