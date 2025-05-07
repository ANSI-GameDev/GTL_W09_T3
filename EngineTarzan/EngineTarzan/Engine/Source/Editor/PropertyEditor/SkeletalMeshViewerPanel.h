#pragma once
#include "Delegates/Delegate.h"
#include "UnrealEd/EditorPanel.h"
#include "HAL/PlatformType.h"

struct FReferenceSkeleton;

class USkeletalMeshViewerPanel : public UEditorPanel
{
public:
    USkeletalMeshViewerPanel();
    ~USkeletalMeshViewerPanel() override;
    void Render() override;
    void OnResize(HWND hWnd) override;
    void DrawBoneNode(const FReferenceSkeleton& RefSkeletal, int32 BoneIndex);

    void SetSkeleton(FReferenceSkeleton* RefSkeletal);
    FReferenceSkeleton* GetSkeleton() const;
    
private:
    float Width = 300, Height = 100;

    FReferenceSkeleton* CurrentRefSkeleton = nullptr;

    static int SelectedBoneIndex;
};
