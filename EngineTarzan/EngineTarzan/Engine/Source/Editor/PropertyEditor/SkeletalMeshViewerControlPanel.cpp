#include "SkeletalMeshViewerControlPanel.h"

#include "SkeletalMeshViewerPanel.h"
#include "Container/String.h"
#include "ImGUI/imgui.h"
#include "LevelEditor/SLevelEditor.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include "UObject/Casts.h"

USkeletalMeshViewerControlPanel::USkeletalMeshViewerControlPanel()
{
}

USkeletalMeshViewerControlPanel::~USkeletalMeshViewerControlPanel()
{
}

void USkeletalMeshViewerControlPanel::Render()
{
    /* Pre Setup */
    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT];
    constexpr ImVec2 IconSize = ImVec2(32, 32);

    const float PanelWidth = (Width) * 0.8f;
    constexpr float PanelHeight = 45.0f;

    constexpr float PanelPosX = 1.0f;
    constexpr float PanelPosY = 1.0f;

    constexpr ImVec2 MinSize(300, 50);
    constexpr ImVec2 MaxSize(FLT_MAX, 50);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    constexpr ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    /* Render Start */
    ImGui::Begin("SkeletalMeshViewerControl Panel", nullptr, PanelFlags);
    
    /* Load SkeletalMesh via native file dialog */
    if (ImGui::Button("Load SkeletalMesh...", IconSize))
    {
        // // 파일 오픈 윈도우
        // const char* filterPatterns[] = { "*.fbx" };
        // const char* FilePath = tinyfd_openFileDialog(
        //     "Open SkeletalMesh Asset", // dialog title
        //     "",                        // initial path
        //     1, filterPatterns,          // filter patterns
        //     "fbx Files",     // filter description
        //     0                           // single selection
        // );
        // if (!FilePath)
        // {
        //     tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
        //     ImGui::End();
        //     return;
        // }
        //
        // // 파일 경로 조정
        // // Full path to asset path conversion
        // FString AssetPath = FilePath;
        // AssetPath = FPaths::ConvertRelativePathToFull(AssetPath);
        // FString ContentDir = FPaths::ProjectContentDir();
        // AssetPath = AssetPath.Replace(*ContentDir, TEXT("/Game/"));
        // AssetPath = AssetPath.Replace(TEXT(".fbx"), TEXT(""));
        //
        // // SkeletalMesh Load 및 ViewPanel에 지정
        // USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *AssetPath));
        // if (LoadedMesh && SkeletalMeshViewerPanel)
        // {
        //     SkeletalMeshViewerPanel->SetSkeleton();
        // }
    }

    // Close 버튼 추가: 클릭 시 패널 숨김 처리
    ImGui::SameLine();
    if (ImGui::Button("Close", IconSize))
    {
        FEngineLoop::GraphicDevice.Resize(GEngineLoop.AppWnd);
        SLevelEditor* LevelEd = GEngineLoop.GetLevelEditor();
        LevelEd->SetSkeletalMeshViewportClient(false);
    }

    
    ImGui::End();
}

void USkeletalMeshViewerControlPanel::OnResize(const HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}
