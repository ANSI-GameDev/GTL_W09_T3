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
    ViewportIndex = static_cast<int32>(InViewportIndex);
    
    PerspectiveCamera.SetLocation(FVector(-10.0f, 0.f, 0.0f));
    PerspectiveCamera.SetRotation(FRotator(0, 0.f, 0.f));
    
    Viewport = new FViewport(InViewportIndex);
    Viewport->Initialize(InRect);

    GizmoActor = FObjectFactory::ConstructObject<ATransformGizmo>(GEngine); // TODO : EditorEngine 외의 다른 Engine 형태가 추가되면 GEngine 대신 다른 방식으로 넣어주어야 함.
    GizmoActor->Initialize(this);
}

void FSkeletalMeshViewportClient::Tick(float DeltaTime)
{
    FEditorViewportClient::Tick(DeltaTime);
}

void FSkeletalMeshViewportClient::InputKey(const FKeyEvent& InKeyEvent)
{
    FEditorViewportClient::InputKey(InKeyEvent);
}
