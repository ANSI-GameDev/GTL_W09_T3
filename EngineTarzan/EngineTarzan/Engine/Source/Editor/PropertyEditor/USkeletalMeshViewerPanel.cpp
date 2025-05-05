#include "USkeletalMeshViewerPanel.h"

#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"

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

    ImGui::End();
}

void USkeletalMeshViewerPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}
