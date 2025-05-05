#pragma once
#include "PropertyEditorPanel.h"

class USkeletalMeshViewerPanel : public UEditorPanel
{
public:
    USkeletalMeshViewerPanel();
    ~USkeletalMeshViewerPanel() override;
    void Render() override;
    void OnResize(HWND hWnd) override;
    void DrawBoneNode();
private:
    float Width = 300, Height = 100;
};
