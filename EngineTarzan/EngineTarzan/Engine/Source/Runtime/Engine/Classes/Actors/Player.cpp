#include "Player.h"

#include "UnrealClient.h"
#include "World/World.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "BaseGizmos/GizmoCircleComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/Light/LightComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"
#include "Engine/EditorEngine.h"

UEditorPlayer::~UEditorPlayer()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
    if (Handler == nullptr) return;
    
    for (const FDelegateHandle& Handle : InputDelegatesHandles)
    {
        Handler->OnKeyCharDelegate.Remove(Handle);
        Handler->OnKeyDownDelegate.Remove(Handle);
        Handler->OnKeyUpDelegate.Remove(Handle);
        Handler->OnMouseDownDelegate.Remove(Handle);
        Handler->OnMouseUpDelegate.Remove(Handle);
        Handler->OnMouseDoubleClickDelegate.Remove(Handle);
        Handler->OnMouseWheelDelegate.Remove(Handle);
        Handler->OnMouseMoveDelegate.Remove(Handle);
        Handler->OnRawMouseInputDelegate.Remove(Handle);
        Handler->OnRawKeyboardInputDelegate.Remove(Handle);
    }
}

void UEditorPlayer::Initialize()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
    if (Handler == nullptr) return;
    
    for (const FDelegateHandle& Handle : InputDelegatesHandles)
    {
        Handler->OnKeyCharDelegate.Remove(Handle);
        Handler->OnKeyDownDelegate.Remove(Handle);
        Handler->OnKeyUpDelegate.Remove(Handle);
        Handler->OnMouseDownDelegate.Remove(Handle);
        Handler->OnMouseUpDelegate.Remove(Handle);
        Handler->OnMouseDoubleClickDelegate.Remove(Handle);
        Handler->OnMouseWheelDelegate.Remove(Handle);
        Handler->OnMouseMoveDelegate.Remove(Handle);
        Handler->OnRawMouseInputDelegate.Remove(Handle);
        Handler->OnRawKeyboardInputDelegate.Remove(Handle);
    }
    
    InputDelegatesHandles.Add(Handler->OnMouseDownDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return;
        if (io.WantCaptureKeyboard) return;

        switch (InMouseEvent.GetEffectingButton())
        {
            case EKeys::LeftMouseButton:
            {
                POINT mousePos;
                GetCursorPos(&mousePos);
                GetCursorPos(&m_LastMousePos);
                ScreenToClient(GEngineLoop.AppWnd, &mousePos);

                FVector pickPosition;

                std::shared_ptr<FEditorViewportClient> ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
                ScreenToViewSpace(mousePos.x, mousePos.y, ActiveViewport, pickPosition);
                bool res = PickGizmo(pickPosition, ActiveViewport.get());
                if (!res) PickActor(pickPosition);
            }
        }
    }));

    InputDelegatesHandles.Add(Handler->OnMouseUpDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        switch (InMouseEvent.GetEffectingButton())
        {
            case EKeys::LeftMouseButton:
            {
                std::shared_ptr<FEditorViewportClient> ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
                ActiveViewport->SetPickedGizmoComponent(nullptr);
            }
        }
    }));

    InputDelegatesHandles.Add(Handler->OnRawMouseInputDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
    {
        
    }));
}

void UEditorPlayer::ProcessGizmoIntersection(UStaticMeshComponent* Component, const FVector& PickPosition, FEditorViewportClient* InActiveViewport, bool& bIsPickedGizmo)
{
    int maxIntersect = 0;
    float minDistance = FLT_MAX;
    float Distance = 0.0f;
    int currentIntersectCount = 0;
    if (!Component) return;
    if (RayIntersectsObject(PickPosition, Component, Distance, currentIntersectCount))
    {
        if (Distance < minDistance)
        {
            InActiveViewport->SetPickedGizmoComponent(Component);
            bIsPickedGizmo = true;
        }
        else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
        {
            InActiveViewport->SetPickedGizmoComponent(Component);
            bIsPickedGizmo = true;
        }
    }
}

bool UEditorPlayer::PickGizmo(const FVector& pickPosition, FEditorViewportClient* InActiveViewport)
{
    bool isPickedGizmo = false;
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (Engine->GetSelectedActor())
    {
        if (ControlMode == CM_TRANSLATION)
        {
            for (UStaticMeshComponent* iter : InActiveViewport->GetGizmoActor()->GetArrowArr())
            {
                ProcessGizmoIntersection(iter, pickPosition, InActiveViewport, isPickedGizmo);
            }
        }
        else if (ControlMode == CM_ROTATION)
        {
            for (UStaticMeshComponent* iter : InActiveViewport->GetGizmoActor()->GetDiscArr())
            {
                ProcessGizmoIntersection(iter, pickPosition, InActiveViewport, isPickedGizmo);
            }
        }
        else if (ControlMode == CM_SCALE)
        {
            for (UStaticMeshComponent* iter : InActiveViewport->GetGizmoActor()->GetScaleArr())
            {
                ProcessGizmoIntersection(iter, pickPosition, InActiveViewport, isPickedGizmo);
            }
        }
    }
    return isPickedGizmo;
}

void UEditorPlayer::PickActor(const FVector& pickPosition)
{
    if (!(ShowFlags::GetInstance().CurrentFlags & EEngineShowFlags::SF_Primitives)) return;

    USceneComponent* Possible = nullptr;
    int maxIntersect = 0;
    float minDistance = FLT_MAX;
    for (const auto iter : TObjectRange<UPrimitiveComponent>())
    {
        if (iter->GetWorld() != GEngine->ActiveWorld)
        {
            continue;
        }
        
        UPrimitiveComponent* pObj;
        if (iter->IsA<UPrimitiveComponent>() || iter->IsA<ULightComponentBase>())
        {
            pObj = static_cast<UPrimitiveComponent*>(iter);
        }
        else
        {
            continue;
        }

        if (pObj && !pObj->IsA<UGizmoBaseComponent>())
        {
            float Distance = 0.0f;
            int currentIntersectCount = 0;
            if (RayIntersectsObject(pickPosition, pObj, Distance, currentIntersectCount))
            {
                if (Distance < minDistance)
                {
                    minDistance = Distance;
                    maxIntersect = currentIntersectCount;
                    Possible = pObj;
                }
                else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                {
                    maxIntersect = currentIntersectCount;
                    Possible = pObj;
                }
            }
        }
    }
    if (Possible)
    {
        Cast<UEditorEngine>(GEngine)->SelectActor(Possible->GetOwner());
        Cast<UEditorEngine>(GEngine)->SelectComponent(Possible);
    }
    else
    {
        Cast<UEditorEngine>(GEngine)->DeselectActor(Cast<UEditorEngine>(GEngine)->GetSelectedActor());
        Cast<UEditorEngine>(GEngine)->DeselectComponent(Cast<UEditorEngine>(GEngine)->GetSelectedComponent());
    }
}

void UEditorPlayer::AddControlMode()
{
    ControlMode = static_cast<EControlMode>((ControlMode + 1) % CM_END);
}

void UEditorPlayer::AddCoordiMode()
{
    CoordMode = static_cast<ECoordMode>((CoordMode + 1) % CDM_END);
}

void UEditorPlayer::ScreenToViewSpace(int32 ScreenX, int32 ScreenY, std::shared_ptr<FEditorViewportClient> ActiveViewport, FVector& RayOrigin)
{
    FRect Rect = ActiveViewport->GetViewport()->GetRect();
    
    float ViewportX = static_cast<float>(ScreenX) - Rect.TopLeftX;
    float ViewportY = static_cast<float>(ScreenY) - Rect.TopLeftY;

    FMatrix ProjectionMatrix = ActiveViewport->GetProjectionMatrix();
    
    RayOrigin.X = ((2.0f * ViewportX / Rect.Width) - 1) / ProjectionMatrix[0][0];
    RayOrigin.Y = -((2.0f * ViewportY / Rect.Height) - 1) / ProjectionMatrix[1][1];
    
    if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->IsOrthographic())
    {
        RayOrigin.Z = 0.0f;  // 오쏘 모드에서는 unproject 시 near plane 위치를 기준
    }
    else
    {
        RayOrigin.Z = 1.0f;  // 퍼스펙티브 모드: near plane
    }
}

int UEditorPlayer::RayIntersectsObject(const FVector& PickPosition, USceneComponent* Component, float& HitDistance, int& IntersectCount)
{
    FMatrix WorldMatrix = Component->GetWorldMatrix();
    FMatrix ViewMatrix = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();
    
    bool bIsOrtho = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->IsOrthographic();
    

    if (bIsOrtho)
    {
        // 오쏘 모드: ScreenToViewSpace()에서 계산된 pickPosition이 클립/뷰 좌표라고 가정
        FMatrix inverseView = FMatrix::Inverse(ViewMatrix);
        // pickPosition을 월드 좌표로 변환
        FVector worldPickPos = inverseView.TransformPosition(PickPosition);  
        // 오쏘에서는 픽킹 원점은 unproject된 픽셀의 위치
        FVector rayOrigin = worldPickPos;
        // 레이 방향은 카메라의 정면 방향 (평행)
        FVector orthoRayDir = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->OrthogonalCamera.GetForwardVector().GetSafeNormal();

        // 객체의 로컬 좌표계로 변환
        FMatrix LocalMatrix = FMatrix::Inverse(WorldMatrix);
        FVector LocalRayOrigin = LocalMatrix.TransformPosition(rayOrigin);
        FVector LocalRayDir = (LocalMatrix.TransformPosition(rayOrigin + orthoRayDir) - LocalRayOrigin).GetSafeNormal();
        
        IntersectCount = Component->CheckRayIntersection(LocalRayOrigin, LocalRayDir, HitDistance);
        return IntersectCount;
    }
    else
    {
        FMatrix inverseMatrix = FMatrix::Inverse(WorldMatrix * ViewMatrix);
        FVector cameraOrigin = { 0,0,0 };
        FVector pickRayOrigin = inverseMatrix.TransformPosition(cameraOrigin);
        // 퍼스펙티브 모드의 기존 로직 사용
        FVector transformedPick = inverseMatrix.TransformPosition(PickPosition);
        FVector rayDirection = (transformedPick - pickRayOrigin).GetSafeNormal();
        
        IntersectCount = Component->CheckRayIntersection(pickRayOrigin, rayDirection, HitDistance);

        if (IntersectCount > 0)
        {
            FVector LocalHitPoint = pickRayOrigin + rayDirection * HitDistance;

            FVector WorldHitPoint = WorldMatrix.TransformPosition(LocalHitPoint);

            FVector WorldRayOrigin;
            FMatrix InverseView = FMatrix::Inverse(ViewMatrix);
            WorldRayOrigin = InverseView.TransformPosition(cameraOrigin);

            float WorldDistance = FVector::Distance(WorldRayOrigin, WorldHitPoint);

            HitDistance = WorldDistance;
        }
        return IntersectCount;
    }
}

UObject* APlayer::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewActor;
}

void APlayer::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
}
