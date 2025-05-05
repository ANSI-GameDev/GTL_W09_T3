#pragma once
#include "EditorViewportClient.h"

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

private:
    //USkeletalMesh* Mesh = nullptr;
    //float AnimTime = 0.0f;
};
 
