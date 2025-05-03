#include "SkeletalMeshComponent.h"

#include "Engine/SkeletalMesh.h"
#include "UnrealEd/FbxImporter.h"
#include "UObject/ObjectFactory.h"

USkeletalMeshComponent::USkeletalMeshComponent()
{
    UE_LOG(LogLevel::Error, TEXT("SkeletalMeshComponent::USkeletalMeshComponent()"));
    SkeletalMesh= FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);
    FFbxImporter::ParseReferenceSkeleton("Contents/FBX/Anime_character.fbx", SkeletalMesh->RefSkeleton);

    FFbxImporter::ParseSkeletalMeshLODModel(SkeletalMesh->ImportedModel, *SkeletalMesh->ImportedModel);

}

void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
