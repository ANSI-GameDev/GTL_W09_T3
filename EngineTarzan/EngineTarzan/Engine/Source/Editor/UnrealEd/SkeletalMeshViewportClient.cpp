#include "SkeletalMeshViewportClient.h"

#include "UnrealClient.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Engine/Engine.h"
#include "PropertyEditor/ShowFlags.h"
#include "World/World.h"

FSkeletalMeshViewportClient::FSkeletalMeshViewportClient()
    : FEditorViewportClient()
{
    using Flags = EEngineShowFlags::Type;
    Flags OnlyPrimsAndShadow = static_cast<Flags>(Flags(EEngineShowFlags::SF_Primitives) | Flags(EEngineShowFlags::SF_Shadow));
    SetShowFlag(OnlyPrimsAndShadow);
    SetViewMode(EViewModeIndex::VMI_Unlit);
    SetViewportType(ELevelViewportType::LVT_Perspective);
}

FSkeletalMeshViewportClient::~FSkeletalMeshViewportClient()
{
}

void FSkeletalMeshViewportClient::Draw(FViewport* Viewport)
{
    FEditorViewportClient::Draw(Viewport);
}

UWorld* FSkeletalMeshViewportClient::GetWorld() const
{
    return FEditorViewportClient::GetWorld();
}

void FSkeletalMeshViewportClient::Initialize(EViewScreenLocation InViewportIndex, const FRect& InRect)
{
    FEditorViewportClient::Initialize(InViewportIndex, InRect);
    
    PerspectiveCamera.SetLocation(FVector(-10.0f, 0.f, 0.0f));
    PerspectiveCamera.SetRotation(FRotator(0, 0.f, 0.f));
}

void FSkeletalMeshViewportClient::Tick(float DeltaTime)
{
    FEditorViewportClient::Tick(DeltaTime);
}

void FSkeletalMeshViewportClient::InputKey(const FKeyEvent& InKeyEvent)
{
    FEditorViewportClient::InputKey(InKeyEvent);
}
