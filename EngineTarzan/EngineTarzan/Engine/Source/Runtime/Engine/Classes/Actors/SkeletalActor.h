#pragma once
#include "GameFramework/Actor.h"
#include "UObject/ObjectMacros.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;

class ASkeletalActor : public AActor
{
    DECLARE_CLASS(ASkeletalActor, AActor)
public:
    ASkeletalActor();
    USkeletalMeshComponent* GetSkeletalMeshComponent() const;
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY
    (USkeletalMeshComponent*, SkeletalMeshComponent, = nullptr);
    TArray<UStaticMeshComponent*> Bones;
};

