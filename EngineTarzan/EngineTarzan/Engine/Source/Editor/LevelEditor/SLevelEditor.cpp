#include "SLevelEditor.h"
#include <fstream>
#include <ostream>
#include <sstream>
#include "EngineLoop.h"
#include "UnrealClient.h"
#include "WindowsCursor.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "Engine/EditorEngine.h"
#include "Slate/Widgets/Layout/SSplitter.h"
#include "SlateCore/Widgets/SWindow.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/SkeletalMeshViewportClient.h"

extern FEngineLoop GEngineLoop;


SLevelEditor::SLevelEditor()
    : HSplitter(nullptr)
    , VSplitter(nullptr)
    , bMultiViewportMode(false)
{
}

void SLevelEditor::Initialize(uint32 InEditorWidth, uint32 InEditorHeight)
{
    ResizeEditor(InEditorWidth, InEditorHeight);
    
    VSplitter = new SSplitterV();
    VSplitter->Initialize(FRect(0.0f, 0.f, InEditorWidth, InEditorHeight));
    
    HSplitter = new SSplitterH();
    HSplitter->Initialize(FRect(0.f, 0.0f, InEditorWidth, InEditorHeight));
    
    FRect Top = VSplitter->SideLT->GetRect();
    FRect Bottom = VSplitter->SideRB->GetRect();
    FRect Left = HSplitter->SideLT->GetRect();
    FRect Right = HSplitter->SideRB->GetRect();

    for (size_t i = 0; i < 4; i++)
    {
        EViewScreenLocation Location = static_cast<EViewScreenLocation>(i);
        FRect Rect;
        switch (Location)
        {
        case EViewScreenLocation::EVL_TopLeft:
            Rect.TopLeftX = Left.TopLeftX;
            Rect.TopLeftY = Top.TopLeftY;
            Rect.Width = Left.Width;
            Rect.Height = Top.Height;
            break;
        case EViewScreenLocation::EVL_TopRight:
            Rect.TopLeftX = Right.TopLeftX;
            Rect.TopLeftY = Top.TopLeftY;
            Rect.Width = Right.Width;
            Rect.Height = Top.Height;
            break;
        case EViewScreenLocation::EVL_BottomLeft:
            Rect.TopLeftX = Left.TopLeftX;
            Rect.TopLeftY = Bottom.TopLeftY;
            Rect.Width = Left.Width;
            Rect.Height = Bottom.Height;
            break;
        case EViewScreenLocation::EVL_BottomRight:
            Rect.TopLeftX = Right.TopLeftX;
            Rect.TopLeftY = Bottom.TopLeftY;
            Rect.Width = Right.Width;
            Rect.Height = Bottom.Height;
            break;
        default:
            return;
        }
        ViewportClients[i] = std::make_shared<FEditorViewportClient>();
        ViewportClients[i]->Initialize(Location, Rect);
    }
    
    ActiveViewportClient = ViewportClients[0];

    SkeletalMeshViewportClient = std::make_shared<FSkeletalMeshViewportClient>();
    EViewScreenLocation Location = EViewScreenLocation::EVL_TopLeft;
    FRect Rect;
    Rect.TopLeftX = 0;
    Rect.TopLeftY = 0;
    Rect.Width = InEditorWidth;
    Rect.Height = InEditorHeight;
    SkeletalMeshViewportClient->Initialize(EViewScreenLocation::EVL_TopLeft, Rect);
    
    LoadConfig();

    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    Handler->OnPIEModeStartDelegate.AddLambda([this]()
        {
            this->RegisterPIEInputDelegates();
        });

    Handler->OnPIEModeEndDelegate.AddLambda([this]()
        {
            this->RegisterEditorInputDelegates();
        });

    // Register Editor input when first initialization. 
    RegisterEditorInputDelegates();

    Handler->OnSkeletalMeshViewerStartDelegate.AddLambda([this]()
    {
        this->RegisterStaticMeshViewerInputDelegates();
        ActiveViewportClient = SkeletalMeshViewportClient;
        ActiveViewportClient->SetCameraSpeed(0.01f);
        ActiveViewportClient->PerspectiveCamera.SetLocation(FVector(10.0f, 0.f, 5.0f));
        ActiveViewportClient->PerspectiveCamera.SetRotation(FVector(0.0f, 0.0f, 180.0f));
        ActiveViewportClient->UpdateViewMatrix();
        ActiveViewportClient->UpdateProjectionMatrix();
        ActiveViewportClient->SetPickedGizmoComponent(nullptr);
    });

    Handler->OnSkeletalMeshViewerEndDelegate.AddLambda([this]()
    {
        SkeletalMeshViewportClient->SetPickedGizmoComponent(nullptr);
        this->RegisterEditorInputDelegates();
        ActiveViewportClient = ViewportClients[0];
    });
}

void SLevelEditor::Tick(float DeltaTime)
{
    for (std::shared_ptr<FEditorViewportClient> Viewport : ViewportClients)
    {
        Viewport->Tick(DeltaTime);
    }

    SkeletalMeshViewportClient->Tick(DeltaTime);
}

void SLevelEditor::Release()
{
    delete VSplitter;
    delete HSplitter;
}

void SLevelEditor::ResizeEditor(uint32 InEditorWidth, uint32 InEditorHeight)
{
    if (InEditorWidth == EditorWidth && InEditorHeight == EditorHeight)
    {
        return;
    }
    
    EditorWidth = InEditorWidth;
    EditorHeight = InEditorHeight;

    if (HSplitter && VSplitter)
    {
        HSplitter->OnResize(EditorWidth, EditorHeight);
        VSplitter->OnResize(EditorWidth, EditorHeight);
        ResizeViewports();
    }
}

void SLevelEditor::SelectViewport(const FVector2D& Point)
{
    for (int i = 0; i < 4; i++)
    {
        if (ViewportClients[i]->IsSelected(Point))
        {
            SetActiveViewportClient(i);
            return;
        }
    }
}

void SLevelEditor::ResizeViewports()
{
    if (bMultiViewportMode)
    {
        if (GetViewports()[0])
        {
            for (int i = 0; i < 4; ++i)
            {
                GetViewports()[i]->ResizeViewport(
                    VSplitter->SideLT->GetRect(),
                    VSplitter->SideRB->GetRect(),
                    HSplitter->SideLT->GetRect(),
                    HSplitter->SideRB->GetRect()
                );
            }
        }
    }
    else
    {
        ActiveViewportClient->GetViewport()->ResizeViewport(FRect(0.0f, 0.0f, EditorWidth, EditorHeight));
        SkeletalMeshViewportClient->GetViewport()->ResizeViewport(FRect(0.0f, 0.0f, EditorWidth, EditorHeight));
    }
}

void SLevelEditor::SetEnableMultiViewport(bool bIsEnable)
{
    bMultiViewportMode = bIsEnable;
    ResizeViewports();
}

bool SLevelEditor::IsMultiViewport() const
{
    return bMultiViewportMode;
}

void SLevelEditor::SetSkeletalMeshViewportClient(const bool bInSkeletalMeshViewMode)
{
    // 멀티뷰포트 모드는 항상 해제
    bMultiViewportMode      = false; 
    // 바로 전달된 값으로 설정
    bSkeletalMeshViewMode   = bInSkeletalMeshViewMode;
    
    if (UEditorEngine* EdEngine = Cast<UEditorEngine>(GEngine))
    {
        if (bSkeletalMeshViewMode)
            EdEngine->OpenSkeletalMeshViewer();
        else
            EdEngine->CloseSkeletalMeshViewer();
    }
    
    ResizeViewports();
}

std::shared_ptr<FSkeletalMeshViewportClient> SLevelEditor::GetSkeletalMeshViewportClient() const
{
    return SkeletalMeshViewportClient;
}

bool SLevelEditor::IsSkeletalMeshViewMode() const
{
    return bSkeletalMeshViewMode;
}

void SLevelEditor::LoadConfig()
{
    auto Config = ReadIniFile(IniFilePath);

    int32 WindowX = FMath::Max(GetValueFromConfig(Config, "WindowX", 0), 0);
    int32 WindowY = FMath::Max(GetValueFromConfig(Config, "WindowY", 0), 0);
    int32 WindowWidth = GetValueFromConfig(Config, "WindowWidth", EditorWidth);
    int32 WindowHeight = GetValueFromConfig(Config, "WindowHeight", EditorHeight);
    if (WindowWidth > 100 && WindowHeight > 100)
    {
        MoveWindow(GEngineLoop.AppWnd, WindowX, WindowY, WindowWidth, WindowHeight, true);
    }
    bool Zoomed = GetValueFromConfig(Config, "Zoomed", false);
    if (Zoomed)
    {
        ShowWindow(GEngineLoop.AppWnd, SW_MAXIMIZE);
    }
    
    FEditorViewportClient::Pivot.X = GetValueFromConfig(Config, "OrthoPivotX", 0.0f);
    FEditorViewportClient::Pivot.Y = GetValueFromConfig(Config, "OrthoPivotY", 0.0f);
    FEditorViewportClient::Pivot.Z = GetValueFromConfig(Config, "OrthoPivotZ", 0.0f);
    FEditorViewportClient::OrthoSize = GetValueFromConfig(Config, "OrthoZoomSize", 10.0f);

    SetActiveViewportClient(GetValueFromConfig(Config, "ActiveViewportIndex", 0));
    bMultiViewportMode = GetValueFromConfig(Config, "bMultiView", false);
    if (bMultiViewportMode)
    {
        SetEnableMultiViewport(true);
    }
    else
    {
        SetEnableMultiViewport(false);
    }
    
    for (size_t i = 0; i < 4; i++)
    {
        ViewportClients[i]->LoadConfig(Config);
    }
    
    if (HSplitter)
    {
        HSplitter->LoadConfig(Config);
    }
    if (VSplitter)
    {
        VSplitter->LoadConfig(Config);
    }

    ResizeViewports();
}

void SLevelEditor::SaveConfig()
{
    TMap<FString, FString> config;
    if (HSplitter)
    {
        HSplitter->SaveConfig(config);
    }
    if (VSplitter)
    {
        VSplitter->SaveConfig(config);
    }
    for (size_t i = 0; i < 4; i++)
    {
        ViewportClients[i]->SaveConfig(config);
    }
    ActiveViewportClient->SaveConfig(config);

    RECT WndRect = {};
    GetWindowRect(GEngineLoop.AppWnd, &WndRect);
    config["WindowX"] = std::to_string(WndRect.left);
    config["WindowY"] = std::to_string(WndRect.top);
    config["WindowWidth"] = std::to_string(WndRect.right - WndRect.left);
    config["WindowHeight"] = std::to_string(WndRect.bottom - WndRect.top);
    config["Zoomed"] = std::to_string(IsZoomed(GEngineLoop.AppWnd));
    
    config["bMultiView"] = std::to_string(bMultiViewportMode);
    config["ActiveViewportIndex"] = std::to_string(ActiveViewportClient->ViewportIndex);
    config["ScreenWidth"] = std::to_string(EditorWidth);
    config["ScreenHeight"] = std::to_string(EditorHeight);
    config["OrthoPivotX"] = std::to_string(ActiveViewportClient->Pivot.X);
    config["OrthoPivotY"] = std::to_string(ActiveViewportClient->Pivot.Y);
    config["OrthoPivotZ"] = std::to_string(ActiveViewportClient->Pivot.Z);
    config["OrthoZoomSize"] = std::to_string(ActiveViewportClient->OrthoSize);
    WriteIniFile(IniFilePath, config);
}

TMap<FString, FString> SLevelEditor::ReadIniFile(const FString& FilePath)
{
    TMap<FString, FString> config;
    std::ifstream file(*FilePath);
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '[' || line[0] == ';')
        {
            continue;
        }
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value))
        {
            config[key] = value;
        }
    }
    return config;
}

void SLevelEditor::WriteIniFile(const FString& FilePath, const TMap<FString, FString>& Config)
{
    std::ofstream file(*FilePath);
    for (const auto& pair : Config)
    {
        file << *pair.Key << "=" << *pair.Value << "\n";
    }
}

void SLevelEditor::RegisterEditorInputDelegates() 
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
    
    // Clear current delegate functions
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
            if (ImGui::GetIO().WantCaptureMouse) return;

            switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
            {
            case EKeys::LeftMouseButton:
            {
                if (const UEditorEngine* EdEngine = Cast<UEditorEngine>(GEngine))
                {
                    if (const AActor* SelectedActor = EdEngine->GetSelectedActor())
                    {
                        USceneComponent* TargetComponent = nullptr;
                        if (USceneComponent* SelectedComponent = EdEngine->GetSelectedComponent())
                        {
                            TargetComponent = SelectedComponent;
                        }
                        else if (AActor* SelectedActor = EdEngine->GetSelectedActor())
                        {
                            TargetComponent = SelectedActor->GetRootComponent();
                        }
                        else
                        {
                            return;
                        }

                        // 초기 Actor와 Cursor의 거리차를 저장
                        const FViewportCamera* ViewTransform = ActiveViewportClient->GetViewportType() == LVT_Perspective
                                                            ? &ActiveViewportClient->PerspectiveCamera
                                                            : &ActiveViewportClient->OrthogonalCamera;

                        FVector RayOrigin, RayDir;
                        ActiveViewportClient->DeprojectFVector2D(FWindowsCursor::GetClientPosition(), RayOrigin, RayDir);

                        const FVector TargetLocation = TargetComponent->GetWorldLocation();
                        const float TargetDist = FVector::Distance(ViewTransform->GetLocation(), TargetLocation);
                        const FVector TargetRayEnd = RayOrigin + RayDir * TargetDist;
                        TargetDiff = TargetLocation - TargetRayEnd;
                    }
                }
                break;
            }
            case EKeys::RightMouseButton:
            {
                if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
                {
                    FWindowsCursor::SetShowMouseCursor(false);
                    MousePinPosition = InMouseEvent.GetScreenSpacePosition();
                }
                break;
            }
            default:
                break;
            }

            // 마우스 이벤트가 일어난 위치의 뷰포트를 선택
            if (bMultiViewportMode)
            {
                POINT Point;
                GetCursorPos(&Point);
                ScreenToClient(GEngineLoop.AppWnd, &Point);
                FVector2D ClientPos = FVector2D{ static_cast<float>(Point.x), static_cast<float>(Point.y) };
                SelectViewport(ClientPos);
                VSplitter->OnPressed({ ClientPos.X, ClientPos.Y });
                HSplitter->OnPressed({ ClientPos.X, ClientPos.Y });
            }
        }));

    InputDelegatesHandles.Add(Handler->OnMouseMoveDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
        {
            if (ImGui::GetIO().WantCaptureMouse) return;

            // Splitter 움직임 로직
            if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
            {
                const auto& [DeltaX, DeltaY] = InMouseEvent.GetCursorDelta();

                bool bSplitterDragging = false;
                if (VSplitter->IsSplitterPressed())
                {
                    VSplitter->OnDrag(FPoint(DeltaX, DeltaY));
                    bSplitterDragging = true;
                }
                if (HSplitter->IsSplitterPressed())
                {
                    HSplitter->OnDrag(FPoint(DeltaX, DeltaY));
                    bSplitterDragging = true;
                }

                if (bSplitterDragging)
                {
                    ResizeViewports();
                }
            }

            // 멀티 뷰포트일 때, 커서 변경 로직
            if (
                bMultiViewportMode
                && !InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)
                && !InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)
                )
            {
                // TODO: 나중에 커서가 Viewport 위에 있을때만 ECursorType::Crosshair로 바꾸게끔 하기
                // ECursorType CursorType = ECursorType::Crosshair;
                ECursorType CursorType = ECursorType::Arrow;
                POINT Point;

                GetCursorPos(&Point);
                ScreenToClient(GEngineLoop.AppWnd, &Point);
                FVector2D ClientPos = FVector2D{ static_cast<float>(Point.x), static_cast<float>(Point.y) };
                const bool bIsVerticalHovered = VSplitter->IsSplitterHovered({ ClientPos.X, ClientPos.Y });
                const bool bIsHorizontalHovered = HSplitter->IsSplitterHovered({ ClientPos.X, ClientPos.Y });

                if (bIsHorizontalHovered && bIsVerticalHovered)
                {
                    CursorType = ECursorType::ResizeAll;
                }
                else if (bIsHorizontalHovered)
                {
                    CursorType = ECursorType::ResizeLeftRight;
                }
                else if (bIsVerticalHovered)
                {
                    CursorType = ECursorType::ResizeUpDown;
                }
                FWindowsCursor::SetMouseCursor(CursorType);
            }
        }));

    InputDelegatesHandles.Add(Handler->OnMouseUpDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
        {
            switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
            {
            case EKeys::RightMouseButton:
            {
                FWindowsCursor::SetShowMouseCursor(true);
                FWindowsCursor::SetPosition(
                    static_cast<int32>(MousePinPosition.X),
                    static_cast<int32>(MousePinPosition.Y)
                );
                return;
            }

            // Viewport 선택 로직
            case EKeys::LeftMouseButton:
            {
                VSplitter->OnReleased();
                HSplitter->OnReleased();
                return;
            }

            default:
                return;
            }
        }));

    InputDelegatesHandles.Add(Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
        {
            ActiveViewportClient->InputKey(InKeyEvent);
        }));

    InputDelegatesHandles.Add(Handler->OnKeyUpDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
        {
            ActiveViewportClient->InputKey(InKeyEvent);
        }));
}

void SLevelEditor::RegisterPIEInputDelegates()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    // Clear current delegate functions
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
    // Add Delegate functions in PIE mode
}

void SLevelEditor::RegisterStaticMeshViewerInputDelegates()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
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

    // Add Delegate functions in StaticMesh Viewer Mode

    InputDelegatesHandles.Add(Handler->OnMouseDownDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
        {
            if (ImGui::GetIO().WantCaptureMouse) return;

            switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
            {
            case EKeys::RightMouseButton:
            {
                if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
                {
                    FWindowsCursor::SetShowMouseCursor(false);
                    MousePinPosition = InMouseEvent.GetScreenSpacePosition();
                }
                break;
            }
            default:
                break;
            }
        
        }));
    
    InputDelegatesHandles.Add(Handler->OnMouseUpDelegate.AddLambda([this](const FPointerEvent& InMouseEvent)
        {
            switch (InMouseEvent.GetEffectingButton())  // NOLINT(clang-diagnostic-switch-enum)
            {
            case EKeys::RightMouseButton:
            {
                FWindowsCursor::SetShowMouseCursor(true);
                FWindowsCursor::SetPosition(static_cast<int32>(MousePinPosition.X), static_cast<int32>(MousePinPosition.Y));
                return;
            }

            default:
                return;
            }
        }));

    InputDelegatesHandles.Add(Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
        {
            SkeletalMeshViewportClient->InputKey(InKeyEvent);
        }));

    InputDelegatesHandles.Add(Handler->OnKeyUpDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
        {
            SkeletalMeshViewportClient->InputKey(InKeyEvent);
        }));
}
