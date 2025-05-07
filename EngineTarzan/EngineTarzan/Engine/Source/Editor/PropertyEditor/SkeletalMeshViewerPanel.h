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

    void SetSkeleton(FReferenceSkeleton* RefSkeletal);
    FReferenceSkeleton* GetSkeleton() const;

    void SetSkeletalActor(ASkeletalActor* Actor);
    ASkeletalActor* GetSkeletalActor() const;
    
private:
    float Width = 300, Height = 100;

    ASkeletalActor* SkeletalActor = nullptr;

    FReferenceSkeleton* CurrentRefSkeleton = nullptr;

    static int SelectedBoneIndex;
};
