#pragma once
#include <memory>

#include "UnrealEd/EditorPanel.h"

class AActor;
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
    float CameraSpeed = 0.0f;
};
