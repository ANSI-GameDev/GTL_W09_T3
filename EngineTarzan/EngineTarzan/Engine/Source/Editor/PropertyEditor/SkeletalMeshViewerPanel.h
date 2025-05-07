#pragma once
#include "UnrealEd/EditorPanel.h"
#include "HAL/PlatformType.h"

class ASkeletalActor;
struct FReferenceSkeleton;

class USkeletalMeshViewerPanel : public UEditorPanel
{
public:
    USkeletalMeshViewerPanel();
    ~USkeletalMeshViewerPanel() override;
    void Render() override;
    void OnResize(HWND hWnd) override;
    void DrawBoneNode(const FReferenceSkeleton& RefSkeletal, int32 BoneIndex);
    
private:
    float Width = 300, Height = 100;
};
