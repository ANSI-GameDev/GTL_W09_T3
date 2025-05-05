#pragma once
#include "UnrealEd/EditorPanel.h"

class USkeletalMeshViewerPanel;

class USkeletalMeshViewerControlPanel : public UEditorPanel
{
public:
    USkeletalMeshViewerControlPanel();
    ~USkeletalMeshViewerControlPanel() override;
    
    void Render() override;
    void OnResize(HWND hWnd) override;
private:
    float Width = 300, Height = 100;
    
    USkeletalMeshViewerPanel* SkeletalMeshViewerPanel;
};
