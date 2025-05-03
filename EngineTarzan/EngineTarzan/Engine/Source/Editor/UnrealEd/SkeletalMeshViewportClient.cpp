#include "SkeletalMeshViewportClient.h"

#include "UnrealClient.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Engine/Engine.h"
#include "World/World.h"

FSkeletalMeshViewportClient::FSkeletalMeshViewportClient()
{
    PerspectiveCamera.SetLocation(FVector(10, 0, 0));
    PerspectiveCamera.SetRotation(FRotator(0, 0, 0));
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
    
    PerspectiveCamera.SetLocation(FVector(8.0f, 8.0f, 8.f));
    PerspectiveCamera.SetRotation(FVector(0.0f, 45.0f, -135.0f));
    
    Viewport = new FViewport(InViewportIndex);
    Viewport->Initialize(InRect);

    GizmoActor = FObjectFactory::ConstructObject<ATransformGizmo>(GEngine); // TODO : EditorEngine 외의 다른 Engine 형태가 추가되면 GEngine 대신 다른 방식으로 넣어주어야 함.
    GizmoActor->Initialize(this);
}

void FSkeletalMeshViewportClient::Tick(float DeltaTime)
{
    if (GEngine->ActiveWorld->WorldType == EWorldType::StaticMeshViewer)
    {
        UpdateEditorCameraMovement(DeltaTime);
    }
    UpdateViewMatrix();
    UpdateProjectionMatrix();
    GizmoActor->Tick(DeltaTime);
}

void FSkeletalMeshViewportClient::InputKey(const FKeyEvent& InKeyEvent)
{
    FEditorViewportClient::InputKey(InKeyEvent);
}
