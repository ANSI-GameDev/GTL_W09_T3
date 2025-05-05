#include "UnrealEd.h"
#include "EditorPanel.h"

#include "PropertyEditor/ControlEditorPanel.h"
#include "PropertyEditor/OutlinerEditorPanel.h"
#include "PropertyEditor/PropertyEditorPanel.h"
#include "PropertyEditor/SkeletalMeshViewerControlPanel.h"
#include "PropertyEditor/SkeletalMeshViewerPanel.h"
#include "World/World.h"

void UnrealEd::Initialize()
{
    auto ControlPanel = std::make_shared<ControlEditorPanel>();
    Panels["ControlPanel"] = ControlPanel;
    
    auto OutlinerPanel = std::make_shared<OutlinerEditorPanel>();
    Panels["OutlinerPanel"] = OutlinerPanel;
    
    auto PropertyPanel = std::make_shared<PropertyEditorPanel>();
    Panels["PropertyPanel"] = PropertyPanel;

    auto SkeletalMeshViewerPanel = std::make_shared<USkeletalMeshViewerPanel>();
    Panels["SkeletalMeshViewerPanel"] = SkeletalMeshViewerPanel;

    auto SkeletalMeshViewerControlPanel = std::make_shared<USkeletalMeshViewerControlPanel>();
    Panels["SkeletalMeshViewerControlPanel"] = SkeletalMeshViewerControlPanel;
}

void UnrealEd::Render() const
{
    if (GEngine->ActiveWorld->WorldType == EWorldType::Editor || GEngine->ActiveWorld->WorldType == EWorldType::PIE)
    {
        Panels["ControlPanel"]->Render();
        Panels["OutlinerPanel"]->Render();
        Panels["PropertyPanel"]->Render();
    }
    else if (GEngine->ActiveWorld->WorldType == EWorldType::SkeletalMeshViewer)
    {
        Panels["SkeletalMeshViewerControlPanel"]->Render();
        Panels["SkeletalMeshViewerPanel"]->Render();
    }
}

void UnrealEd::AddEditorPanel(const FString& PanelId, const std::shared_ptr<UEditorPanel>& EditorPanel)
{
    Panels[PanelId] = EditorPanel;
}

void UnrealEd::OnResize(HWND hWnd) const
{
    for (auto& Panel : Panels)
    {
        Panel.Value->OnResize(hWnd);
    }
}

std::shared_ptr<UEditorPanel> UnrealEd::GetEditorPanel(const FString& PanelId)
{
    return Panels[PanelId];
}
