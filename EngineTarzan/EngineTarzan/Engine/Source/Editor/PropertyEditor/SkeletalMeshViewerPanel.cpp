#include "SkeletalMeshViewerPanel.h"

#include "ReferenceSkeleton.h"
#include "Actors/SkeletalActor.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/ImGuiWidget.h"
#include "UnrealEd/SkeletalMeshViewportClient.h"

USkeletalMeshViewerPanel::USkeletalMeshViewerPanel()
{
}

USkeletalMeshViewerPanel::~USkeletalMeshViewerPanel()
{
}

void USkeletalMeshViewerPanel::Render()
{
    SLevelEditor* LevelEd = GEngineLoop.GetLevelEditor();
    std::shared_ptr<FSkeletalMeshViewportClient> SkeletalMeshViewportClient = LevelEd->GetSkeletalMeshViewportClient();
    /* Pre Setup */
    const float PanelWidth  = Width * 0.2f - 6.0f;
    const float PanelHeight = Height * 0.65f;

    /* 우측 상단 위치 계산 */
    constexpr float padding = 5.0f;
    const float PanelPosX = Width - PanelWidth - padding;  // 화면 폭에서 패널 폭과 여백을 뺀 값
    const float PanelPosY = padding;                      // 상단 여백만큼 고정

    const ImVec2 MinSize(140, 370);
    const ImVec2 MaxSize(FLT_MAX, 900);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    
    // gizmo 위치·회전 갱신 람다
    ASkeletalActor* skeletalActor = nullptr;
    USkeletalMesh* skeletalMesh = nullptr;

    skeletalActor = SkeletalMeshViewportClient->GetSkeletalActor();
    if (skeletalActor && !skeletalActor->IsActorBeingDestroyed())
    {
        skeletalMesh = skeletalActor->GetSkeletalMeshComponent()->GetSkeletalMesh();
    }
    
    auto UpdateActor = [&](AActor* Gizmo)
    {
        if (!Gizmo) return;
        
        if (skeletalMesh)
        {
            FTransform WorldTransform = skeletalActor->GetSkeletalMeshComponent()->GetBoneWorldTransform(SkeletalMeshViewportClient->GetSelectedBoneIndex());
            Gizmo->SetActorLocation(WorldTransform.GetPosition());
            Gizmo->SetActorRotation(WorldTransform.GetRotation());
        }
    };
    
    if (skeletalMesh == nullptr)
    {
        return;
    }
    
    if (skeletalMesh)
    {
        /* Render Start */
        ImGui::Begin("BoneTree", nullptr, PanelFlags);

        FReferenceSkeleton RefSkel = skeletalMesh->RefSkeleton;
        // 트리 구조 렌더링
        for (int32 i = 0; i < RefSkel.GetNumBones(); ++i)
        {
            if (RefSkel.GetBoneInfo()[i].ParentIndex == INDEX_NONE)
                DrawBoneNode(RefSkel, i);
        }
        
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        
        // 선택된 본 정보 출력
        if (SkeletalMeshViewportClient->GetSelectedBoneIndex() != INDEX_NONE)
        {
            SLevelEditor* LevelEditor = GEngineLoop.GetLevelEditor();
            if (const std::shared_ptr<FEditorViewportClient> EditorViewportClient = LevelEditor->GetActiveViewportClient())
            {
                UpdateActor(EditorViewportClient->GetGizmoActor());
            }

            UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
            if (skeletalActor != nullptr || skeletalActor->IsActorBeingDestroyed())
            {
                Engine->SelectActor(Cast<AActor>(skeletalActor));
            }
            
            FTransform Pose = skeletalActor->GetSkeletalMeshComponent()->GetBoneWorldTransform(SkeletalMeshViewportClient->GetSelectedBoneIndex());

            FVector Pos   = Pose.GetPosition();
            FRotator Rot  = Pose.GetRotation();
            FVector Scale = Pose.GetScale();

            FImGuiWidget::DrawVec3Control("Location",  Pos, 0, 85);
            ImGui::Spacing();
            
            FImGuiWidget::DrawRot3Control("Rotation", Rot, 0, 85);
            ImGui::Spacing();
            
            FImGuiWidget::DrawVec3Control("Scale",  Scale, 0, 85);
            ImGui::Spacing();            

            FTransform Updated = FTransform(Pos, Rot, Scale);
            skeletalActor->GetSkeletalMeshComponent()->SetBoneWorldTransform(SkeletalMeshViewportClient->GetSelectedBoneIndex(), Updated);
        }
        ImGui::PopStyleColor();
        ImGui::End();
    }
}

void USkeletalMeshViewerPanel::OnResize(const HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void USkeletalMeshViewerPanel::DrawBoneNode(const FReferenceSkeleton& RefSkeletal, const int32 BoneIndex)
{
    SLevelEditor* LevelEd = GEngineLoop.GetLevelEditor();
    std::shared_ptr<FSkeletalMeshViewportClient> SkeletalMeshViewportClient = LevelEd->GetSkeletalMeshViewportClient();

    
    // 본 이름 UTF-8 변환
    const FMeshBoneInfo& BoneInfo = RefSkeletal.GetBoneInfo()[BoneIndex];
    const FString BoneName = BoneInfo.Name.ToString();
    
    // 이 본의 자식 인덱스를 수집
    TArray<int32> Children;
    for (int32 i = 0; i < RefSkeletal.GetNumBones(); ++i)
    {
        if (RefSkeletal.GetBoneInfo()[i].ParentIndex == BoneIndex)
        {
            Children.Add(i);
        }
    }

    bool bHasChildren = Children.Num() > 0;

    // 트리 노드 플래그 설정: 항상 펼치고 전체 너비 사용
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
    flags |= (bHasChildren ? ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf);
    if (BoneIndex == SkeletalMeshViewportClient->GetSelectedBoneIndex())
        flags |= ImGuiTreeNodeFlags_Selected;
    
    // BoneIndex를 ID로 사용하고 이름 표시
    const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(BoneIndex)), flags, "%s", GetData(BoneName));
    if (ImGui::IsItemClicked())
        SkeletalMeshViewportClient->SetSelectedBoneIndex(BoneIndex);

    if (opened)
    {
        for (const int32 ChildIndex : Children)
        {
            DrawBoneNode(RefSkeletal, ChildIndex);
        }
        ImGui::TreePop();
    }
}
