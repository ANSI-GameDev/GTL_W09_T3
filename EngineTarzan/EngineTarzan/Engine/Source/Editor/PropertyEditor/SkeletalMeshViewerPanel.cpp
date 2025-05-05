#include "SkeletalMeshViewerPanel.h"

#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"

// 선택된 본 인덱스 저장
static int SelectedBoneIndex = INDEX_NONE;

USkeletalMeshViewerPanel::USkeletalMeshViewerPanel()
{
}

USkeletalMeshViewerPanel::~USkeletalMeshViewerPanel()
{
}

void USkeletalMeshViewerPanel::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        return;
    }
    
    /* Pre Setup */
    float PanelWidth  = Width * 0.2f - 6.0f;
    float PanelHeight = Height * 0.65f;

    /* 우측 상단 위치 계산 */
    const float padding = 5.0f;
    float PanelPosX = Width - PanelWidth - padding;  // 화면 폭에서 패널 폭과 여백을 뺀 값
    float PanelPosY = padding;                      // 상단 여백만큼 고정

    ImVec2 MinSize(140, 370);
    ImVec2 MaxSize(FLT_MAX, 900);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    /* Render Start */
    ImGui::Begin("BoneTree", nullptr, PanelFlags);

    // if (CurrentRefSkeleton)
    // {
    //     const FReferenceSkeleton& RefSkel = *CurrentRefSkeleton;
    //     // 트리 구조 렌더링
    //     for (int32 i = 0; i < RefSkel.GetNumBones(); ++i)
    //     {
    //         if (RefSkel.GetBoneInfo()[i].ParentIndex == INDEX_NONE)
    //             DrawBoneNode(RefSkel, i);
    //     }
    //
    //     ImGui::Separator();
    //
    //     // 선택된 본 정보 출력
    //     if (SelectedBoneIndex != INDEX_NONE)
    //     {
    //         const FMeshBoneInfo& Info = RefSkel.GetBoneInfo()[SelectedBoneIndex];
    //         const FTransform& Pose  = RefSkel.GetBonePose()[SelectedBoneIndex];
    //
    //         FVector Pos   = Pose.GetTranslation();
    //         FRotator Rot  = Pose.GetRotation().Rotator();
    //         FVector Scale = Pose.GetScale3D();
    //
    //         ImGui::Text("본 이름: %s", TCHAR_TO_UTF8(*Info.Name.ToString()));
    //         ImGui::Text("위치: %.3f, %.3f, %.3f", Pos.X,   Pos.Y,   Pos.Z);
    //         ImGui::Text("회전: %.3f, %.3f, %.3f", Rot.Pitch, Rot.Yaw, Rot.Roll);
    //         ImGui::Text("스케일: %.3f, %.3f, %.3f", Scale.X, Scale.Y, Scale.Z);
    //     }
    // }
    
    ImGui::End();
}

void USkeletalMeshViewerPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void USkeletalMeshViewerPanel::DrawBoneNode(const FReferenceSkeleton& RefSkeletal, int32 BoneIndex)
{
    // 예시 코드
    // // 본 이름 UTF-8 변환
    // const FMeshBoneInfo& BoneInfo = RefSkeletal.GetBoneInfo()[BoneIndex];
    // const char* BoneName = TCHAR_TO_UTF8(*BoneInfo.Name.ToString());
    //
    // // 이 본의 자식 인덱스를 수집
    // TArray<int32> Children;
    // for (int32 i = 0; i < RefSkeletal.GetNumBones(); ++i)
    // {
    //     if (RefSkeletal.GetBoneInfo()[i].ParentIndex == BoneIndex)
    //     {
    //         Children.Add(i);
    //     }
    // }
    //
    // // 트리 노드 플래그: 화살표, 선택 상태, 리프 노드
    // ImGuiTreeNodeFlags flags = (Children.Num() > 0 ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf)
    //     | (BoneIndex == SelectedBoneIndex ? ImGuiTreeNodeFlags_Selected : 0);
    //
    // // BoneIndex를 ID로 사용하고 이름 표시
    // bool opened = ImGui::TreeNodeEx((void*)(intptr_t)BoneIndex, flags, "%s", BoneName);
    // if (ImGui::IsItemClicked())
    //     SelectedBoneIndex = BoneIndex;
    //
    // bool opened = ImGui::TreeNodeEx(BoneName, flags);
    // if (opened)
    // {
    //     for (const int32 ChildIndex : Children)
    //     {
    //         DrawBoneNode(RefSkeletal, ChildIndex);
    //     }
    //     ImGui::TreePop();
    // }
}

void USkeletalMeshViewerPanel::SetSkeleton(FReferenceSkeleton* RefSkeletal)
{
    CurrentRefSkeleton = RefSkeletal;
}

FReferenceSkeleton* USkeletalMeshViewerPanel::GetSkeleton() const
{
    return CurrentRefSkeleton;
}
