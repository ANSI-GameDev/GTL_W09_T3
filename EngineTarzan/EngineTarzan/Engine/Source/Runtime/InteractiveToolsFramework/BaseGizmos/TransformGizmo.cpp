#include "TransformGizmo.h"
#include "GizmoArrowComponent.h"
#include "Define.h"
#include "GizmoCircleComponent.h"
#include "Actors/Player.h"
#include "GizmoRectangleComponent.h"
#include "World/World.h"
#include "Engine/FObjLoader.h"

ATransformGizmo::ATransformGizmo()
{
    static int a = 0;
    UE_LOG(LogLevel::Error, "Gizmo Created %d", a++);
    FObjManager::CreateStaticMesh("Assets/GizmoTranslationX.obj");
    FObjManager::CreateStaticMesh("Assets/GizmoTranslationY.obj");
    FObjManager::CreateStaticMesh("Assets/GizmoTranslationZ.obj");
    FObjManager::CreateStaticMesh("Assets/GizmoRotationX.obj");
    FObjManager::CreateStaticMesh("Assets/GizmoRotationY.obj");
    FObjManager::CreateStaticMesh("Assets/GizmoRotationZ.obj");
    FObjManager::CreateStaticMesh("Assets/GizmoScaleX.obj");
    FObjManager::CreateStaticMesh("Assets/GizmoScaleY.obj");
    FObjManager::CreateStaticMesh("Assets/GizmoScaleZ.obj");

    SetRootComponent(
        AddComponent<USceneComponent>()
    );

    UGizmoArrowComponent* locationX = AddComponent<UGizmoArrowComponent>();
    locationX->SetStaticMesh(FObjManager::GetStaticMesh(L"Assets/GizmoTranslationX.obj"));
    locationX->SetupAttachment(RootComponent);
    locationX->SetGizmoType(UGizmoBaseComponent::ArrowX);
    ArrowArr.Add(locationX);

    UGizmoArrowComponent* locationY = AddComponent<UGizmoArrowComponent>();
    locationY->SetStaticMesh(FObjManager::GetStaticMesh(L"Assets/GizmoTranslationY.obj"));
    locationY->SetupAttachment(RootComponent);
    locationY->SetGizmoType(UGizmoBaseComponent::ArrowY);
    ArrowArr.Add(locationY);

    UGizmoArrowComponent* locationZ = AddComponent<UGizmoArrowComponent>();
    locationZ->SetStaticMesh(FObjManager::GetStaticMesh(L"Assets/GizmoTranslationZ.obj"));
    locationZ->SetupAttachment(RootComponent);
    locationZ->SetGizmoType(UGizmoBaseComponent::ArrowZ);
    ArrowArr.Add(locationZ);

    UGizmoRectangleComponent* ScaleX = AddComponent<UGizmoRectangleComponent>();
    ScaleX->SetStaticMesh(FObjManager::GetStaticMesh(L"Assets/GizmoScaleX.obj"));
    ScaleX->SetupAttachment(RootComponent);
    ScaleX->SetGizmoType(UGizmoBaseComponent::ScaleX);
    RectangleArr.Add(ScaleX);

    UGizmoRectangleComponent* ScaleY = AddComponent<UGizmoRectangleComponent>();
    ScaleY->SetStaticMesh(FObjManager::GetStaticMesh(L"Assets/GizmoScaleY.obj"));
    ScaleY->SetupAttachment(RootComponent);
    ScaleY->SetGizmoType(UGizmoBaseComponent::ScaleY);
    RectangleArr.Add(ScaleY);

    UGizmoRectangleComponent* ScaleZ = AddComponent<UGizmoRectangleComponent>();
    ScaleZ->SetStaticMesh(FObjManager::GetStaticMesh(L"Assets/GizmoScaleZ.obj"));
    ScaleZ->SetupAttachment(RootComponent);
    ScaleZ->SetGizmoType(UGizmoBaseComponent::ScaleZ);
    RectangleArr.Add(ScaleZ);

    UGizmoCircleComponent* CircleX = AddComponent<UGizmoCircleComponent>();
    CircleX->SetStaticMesh(FObjManager::GetStaticMesh(L"Assets/GizmoRotationX.obj"));
    CircleX->SetupAttachment(RootComponent);
    CircleX->SetGizmoType(UGizmoBaseComponent::CircleX);
    CircleArr.Add(CircleX);

    UGizmoCircleComponent* CircleY = AddComponent<UGizmoCircleComponent>();
    CircleY->SetStaticMesh(FObjManager::GetStaticMesh(L"Assets/GizmoRotationY.obj"));
    CircleY->SetupAttachment(RootComponent);
    CircleY->SetGizmoType(UGizmoBaseComponent::CircleY);
    CircleArr.Add(CircleY);

    UGizmoCircleComponent* CircleZ = AddComponent<UGizmoCircleComponent>();
    CircleZ->SetStaticMesh(FObjManager::GetStaticMesh(L"Assets/GizmoRotationZ.obj"));
    CircleZ->SetupAttachment(RootComponent);
    CircleZ->SetGizmoType(UGizmoBaseComponent::CircleZ);
    CircleArr.Add(CircleZ);
}

void ATransformGizmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATransformGizmo::Initialize(FEditorViewportClient* InViewport)
{
    AttachedViewport = InViewport;
}
