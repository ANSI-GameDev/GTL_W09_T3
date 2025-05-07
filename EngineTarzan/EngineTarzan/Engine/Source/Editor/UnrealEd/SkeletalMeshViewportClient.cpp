#include "SkeletalMeshViewportClient.h"

#include "UnrealClient.h"
#include "WindowsCursor.h"
#include "Actors/SkeletalActor.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "LevelEditor/SLevelEditor.h"
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
}

void FSkeletalMeshViewportClient::Tick(float DeltaTime)
{
    FEditorViewportClient::Tick(DeltaTime);
}

void FSkeletalMeshViewportClient::InputKey(const FKeyEvent& InKeyEvent)
{
    FEditorViewportClient::InputKey(InKeyEvent);
}

void FSkeletalMeshViewportClient::HandleGizmoControl(const FPointerEvent& InMouseEvent)
{
    if (UEditorEngine* EdEngine = Cast<UEditorEngine>(GEngine))
    {
        if (SkeletalActor == nullptr || SkeletalActor->IsActorBeingDestroyed()) return;
        
        USkeletalMesh* skeletalMesh = SkeletalActor->GetSkeletalMeshComponent()->GetSkeletalMesh();
        if (skeletalMesh == nullptr) return;

        // 선택된 본 인덱스
        const int32 BoneIndex = SelectedBoneIndex;
        if (BoneIndex < 0)
        {
            return;
        }

        USkeletalMeshComponent* skeletalMeshComp = SkeletalActor->GetSkeletalMeshComponent();
        // 원래 로컬 트랜스폼 보관
        const FTransform BoneLocalTransform = skeletalMeshComp->GetBoneLocalTransform(BoneIndex);
        const FVector OriginalLocalPos   = BoneLocalTransform.GetPosition();
        const FQuat   OriginalLocalQuat  = BoneLocalTransform.GetRotation().ToQuaternion();
        const FVector OriginalLocalScale = BoneLocalTransform.GetScale();
        
        // 카메라 정보
        const FViewportCamera& Cam = (GetViewportType() == LVT_Perspective) ? PerspectiveCamera : OrthogonalCamera;

        // 스크린 → 월드 레이
        FVector RayOrigin, RayDir;
        DeprojectFVector2D(FWindowsCursor::GetClientPosition(), RayOrigin, RayDir);

        const FVector BoneWorldPos = skeletalMeshComp->GetBoneWorldTransform(BoneIndex).GetPosition();
        float Dist     = FVector::Distance(Cam.GetLocation(), BoneWorldPos);
        FVector RayEnd = RayOrigin + RayDir * Dist;
        FVector Desired = RayEnd;
        
        // 마우스 드래그량
        const FVector2D Delta2D = InMouseEvent.GetCursorDelta();

        // 축 벡터
        const FVector Forward = BoneLocalTransform.GetForward();
        const FVector Right   = BoneLocalTransform.GetRight();
        const FVector Up      = BoneLocalTransform.GetUp();

        // 민감도 설정
        constexpr float TranslSensitivity  = 0.02f;
        constexpr float RotSensitivity     = 0.2f;
        constexpr float ScaleSensitivity   = 0.005f;

        // 델타 계산
        FVector   DeltaTrans(0);
        FRotator  DeltaRot = FRotator(0, 0, 0);
        FVector   DeltaScale(0);

        const UGizmoBaseComponent* Gizmo = Cast<UGizmoBaseComponent>(GetPickedGizmoComponent());
        if (!Gizmo) return;

        switch (Gizmo->GetGizmoType())
        {
            // --- 로컬 축 이동 ---
        case UGizmoBaseComponent::ArrowX:
            DeltaTrans = Forward * FVector::DotProduct(Desired - OriginalLocalPos, Forward) * TranslSensitivity;
            break;
        case UGizmoBaseComponent::ArrowY:
            DeltaTrans = Right   * (Delta2D.Y * TranslSensitivity) * TranslSensitivity;
            break;
        case UGizmoBaseComponent::ArrowZ:
            DeltaTrans = Up      * (Delta2D.Y * TranslSensitivity) * TranslSensitivity;
            break;

            // --- 회전 ---
        case UGizmoBaseComponent::CircleX:
            DeltaRot.Roll  = Delta2D.Y * RotSensitivity;   // 로컬 X축 회전
            break;
        case UGizmoBaseComponent::CircleY:
            DeltaRot.Pitch = Delta2D.Y * RotSensitivity;   // 로컬 Y축 회전
            break;
        case UGizmoBaseComponent::CircleZ:
            DeltaRot.Yaw   = Delta2D.Y * RotSensitivity;   // 로컬 Z축 회전
            break;

            // --- 스케일 ---
        case UGizmoBaseComponent::ScaleX:
            DeltaScale.X = Delta2D.Y * ScaleSensitivity;
            break;
        case UGizmoBaseComponent::ScaleY:
            DeltaScale.Y = Delta2D.Y * ScaleSensitivity;
            break;
        case UGizmoBaseComponent::ScaleZ:
            DeltaScale.Z = Delta2D.Y * ScaleSensitivity;
            break;

        default:
            return;
        }

        if (!DeltaTrans.IsZero())
        {
            skeletalMeshComp->TranslateBone(BoneIndex, DeltaTrans);
        }
        
        if (!DeltaRot.IsZero())
        {
            skeletalMeshComp->RotateBone(BoneIndex, DeltaRot);
        }

        if (!DeltaScale.IsZero())
        {
            skeletalMeshComp->ScaleBone(BoneIndex, DeltaScale);   
        }
        
        // → 여기에 추가: 기즈모 액터도 따라오게
        if (AActor* GizmoActor = Gizmo->GetOwner())
        {
            const FVector MovedBoneWorldPos = skeletalMeshComp->GetBoneWorldTransform(BoneIndex).GetPosition();
            GizmoActor->SetActorLocation(MovedBoneWorldPos);
        }
    }
}
