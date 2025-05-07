#include "SkeletalMeshViewerControlPanel.h"

#include "SkeletalMeshViewerPanel.h"
#include "Components/SkeletalMeshComponent.h"
#include "Container/String.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "ImGUI/imgui.h"
#include "LevelEditor/SLevelEditor.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/FbxImporter.h"
#include "UObject/Casts.h"

USkeletalMeshViewerControlPanel::USkeletalMeshViewerControlPanel()
{
}

USkeletalMeshViewerControlPanel::~USkeletalMeshViewerControlPanel()
{
}

void USkeletalMeshViewerControlPanel::Initialize(const std::shared_ptr<USkeletalMeshViewerPanel>& InSkeletalMeshViewerPanel)
{
    SkeletalMeshViewerPanel = InSkeletalMeshViewerPanel;
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
        // 파일 오픈 윈도우
        const char* filterPatterns[] = { "*.fbx" };
        const char* FilePath = tinyfd_openFileDialog(
            "Open SkeletalMesh Asset", // dialog title
            "",                        // initial path
            1, filterPatterns,          // filter patterns
            "fbx Files",     // filter description
            0                           // single selection
        );
        if (!FilePath)
        {
            tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
            ImGui::End();
            return;
        }

        AActor* skeletalActor = GEngine->ActiveWorld->SpawnActor<AActor>();
        skeletalActor->SetActorLocation(FVector(0,0,0));
        skeletalActor->SetActorRotation(FRotator(0,0,0));
        
        USkeletalMeshComponent* skeletalMeshComp = skeletalActor->AddComponent<USkeletalMeshComponent>();

        USkeletalMesh* skeletalMesh = FObjectFactory::ConstructObject<USkeletalMesh>(nullptr);
        skeletalMesh->Initialize(); // ImportedModel과 SkelMeshRenderData 생성
        
        skeletalMeshComp->SetSkeletalMesh(skeletalMesh);

        FSkeletalMeshLODModel TestSkMeshModel;
        FReferenceSkeleton* TestSkeleton = new FReferenceSkeleton();
        // TODO : 파일 로드 로직
        FFbxImporter::ParseSkeletalMeshLODModel(FilePath, TestSkMeshModel, TestSkeleton);
        
        SkeletalMeshViewerPanel->SetSkeleton(TestSkeleton);
    }
    
    ImGui::SameLine();

    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9c4", IconSize)) // Slider
    {
        ImGui::OpenPopup("SliderControl");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("SliderControl"))
    {
        ImGui::Text("Camera Speed");
        CameraSpeed = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##CamSpeed", &CameraSpeed, 0.01f, 0.198f, 192.0f, "%.2f"))
        {
            GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetCameraSpeed(CameraSpeed);
        }
        ImGui::EndPopup();
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
