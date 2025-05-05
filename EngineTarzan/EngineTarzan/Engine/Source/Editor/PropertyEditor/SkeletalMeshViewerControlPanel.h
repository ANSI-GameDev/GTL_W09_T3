#pragma once
#include <memory>

#include "UnrealEd/EditorPanel.h"

class USkeletalMeshViewerPanel;

class USkeletalMeshViewerControlPanel : public UEditorPanel
{
public:
    USkeletalMeshViewerControlPanel();
    ~USkeletalMeshViewerControlPanel() override;

    void Initialize(const std::shared_ptr<USkeletalMeshViewerPanel>& InSkeletalMeshViewerPanel);
    void Render() override;
    void OnResize(HWND hWnd) override;
private:
    float Width = 300, Height = 100;
    
    std::shared_ptr<USkeletalMeshViewerPanel> SkeletalMeshViewerPanel;
};
