#pragma once
#include "EditorViewportClient.h"

class ASkeletalActor;

class FSkeletalMeshViewportClient : public FEditorViewportClient
{
public:
    FSkeletalMeshViewportClient();
    ~FSkeletalMeshViewportClient() override;
    void Draw(FViewport* Viewport) override;
    UWorld* GetWorld() const override;
    void Initialize(EViewScreenLocation InViewportIndex, const FRect& InRect) override;
    void Tick(float DeltaTime) override;
    void InputKey(const FKeyEvent& InKeyEvent) override;

    void SetSkeletalActor(ASkeletalActor* InSkeletalActor) { SkeletalActor = InSkeletalActor; }

    int GetSelectedBoneIndex() const { return SelectedBoneIndex; }
    void SetSelectedBoneIndex(const int InSelectedBoneIndex) { SelectedBoneIndex = InSelectedBoneIndex; }

    ASkeletalActor* GetSkeletalActor() const { return SkeletalActor; }

private:
    void HandleGizmoControl(const FPointerEvent& InMouseEvent) override;

    ASkeletalActor* SkeletalActor = nullptr;
    
    int SelectedBoneIndex = INDEX_NONE;
    
    //USkeletalMesh* Mesh = nullptr;
    //float AnimTime = 0.0f;
};
 
