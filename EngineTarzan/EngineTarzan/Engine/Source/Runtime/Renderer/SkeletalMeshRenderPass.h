#pragma once
#include "IRenderPass.h"
#include "EngineBaseTypes.h"
#include "Container/Set.h"

#include "Define.h"
#include "Components/Light/PointLightComponent.h"

struct FStaticMeshRenderData;
struct FSkeletalMeshRenderData;
class FShadowManager;
class FDXDShaderManager;
class UWorld;
class UMaterial;
class FEditorViewportClient;
struct FStaticMaterial;
class FShadowRenderPass;
class USkeletalMeshComponent;

// TODO : 우선 StaticMesh Render 정보로 렌더링
//        이후 SkeletalMesh Render 정보로 렌더링하도록 변경

class FSkeletalMeshRenderPass : public IRenderPass
{
public:
    FSkeletalMeshRenderPass();

    virtual ~FSkeletalMeshRenderPass();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;

    void InitializeShadowManager(class FShadowManager* InShadowManager);

    virtual void PrepareRenderArr() override;

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void ClearRenderArr() override;
    void RenderAllSkeletalMeshesForPointLight(const std::shared_ptr<FEditorViewportClient>& Viewport, UPointLightComponent*& PointLight);

    virtual void PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport);

    virtual void RenderAllSkeletalMeshes(const std::shared_ptr<FEditorViewportClient>& Viewport);

    void UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const;

    void UpdateLitUnlitConstant(int32 isLit) const;

    void RenderSkeletalPrimitive(FSkeletalMeshRenderData* RenderData) const;

    // Shader 관련 함수 (생성/해제 등)
    void CreateShader();
    void ReleaseShader();

    void ChangeViewMode(EViewModeIndex ViewMode);

protected:


    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;

    /*
    ID3D11VertexShader* VertexShader;
    ID3D11InputLayout* InputLayout;

    ID3D11PixelShader* PixelShader;
    ID3D11PixelShader* DebugDepthShader;
    ID3D11PixelShader* DebugWorldNormalShader;
    */

    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

    FShadowManager* ShadowManager;
};
