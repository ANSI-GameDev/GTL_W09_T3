#pragma once

#include "StaticMeshRenderPass.h"


class FDepthPrePass : public FStaticMeshRenderPass
{
    friend class FRenderer; // 렌더러에서 접근 가능
    friend class DepthBufferDebugPass; // DepthBufferDebugPass에서 접근 가능
public:
    FDepthPrePass();
    ~FDepthPrePass();
    
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;
    void InitializeSkeletalRenderPass(FSkeletalMeshRenderPass* InSkeletalMeshRenderPass);
    virtual void PrepareRenderArr() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;

    // Begin FStaticMeshRenderPass override
    virtual void PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void RenderAllSkeletalMeshes(const std::shared_ptr<FEditorViewportClient>& Viewport);
    // End FStaticMeshRenderPass override

    FSkeletalMeshRenderPass* SkeletalMeshRenderPass;
};

