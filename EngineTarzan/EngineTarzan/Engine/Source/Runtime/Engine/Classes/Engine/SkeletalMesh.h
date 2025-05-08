#pragma once
#include "ReferenceSkeleton.h"
#include "Asset/UAsset.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

struct FSkeletalMeshRenderData;
// ---------------------------
// Skeletal Mesh용 데이터
// ---------------------------
/**
 * FSkeletalAssetData:
 * FBX Importer로 파싱된 LOD 모델과 본 계층 정보를 담습니다.
 */
struct FSkeletalAssetData : public FAssetData, public std::enable_shared_from_this<FSkeletalAssetData>
{
    // 에디터 Import 단계에서 파싱된 로우 LOD 모델 데이터
    /** RawData 저장용 Imported Model [Unreal에선 LODModel 배열로 관리] */
    FSkeletalMeshLODModel LODModel;
    
    /** 계층구조, Bone Info 및 Transform 배열 저장 */
    FReferenceSkeleton RefSkeleton;

    FSkeletalAssetData() = default;
};



class USkeletalMeshAsset : public UAsset
{
    DECLARE_CLASS(USkeletalMeshAsset, UAsset)
public:
    USkeletalMeshAsset();
    ~USkeletalMeshAsset() override;
    
    UObject* Duplicate(UObject* InOuter) override;
    
    bool InitializeFromData(std::shared_ptr<FAssetData> Data) override;
    void Unload() override;
    EAssetType GetAssetType() const override;
    
    void Initialize();

    void SetImportedModel(const std::shared_ptr<FSkeletalMeshLODModel>& InModel);
    void SetSkeletalMeshRenderData(std::shared_ptr<FSkeletalMeshRenderData> InRenderData);
    void SetRefSkeleton(const FReferenceSkeleton& InRefSkeleton);
    
    std::shared_ptr<FSkeletalMeshLODModel> GetImportedModel() const;
    const FReferenceSkeleton& GetRefSkeleton() const;

    TArray<FStaticMaterial*>& GetMaterials() { return Materials; }
    std::shared_ptr<FSkeletalMeshRenderData> GetSkeletalMeshRenderData() const { return SkelMeshRenderData; }
    
    std::shared_ptr<FSkeletalAssetData> AssetData;

    /** RawData 저장용 Imported Model [Unreal에선 LODModel 배열로 관리] */
    std::shared_ptr<FSkeletalMeshLODModel> ImportedModel;

    /** 런타임용 렌더링 리소스 */
    std::shared_ptr<FSkeletalMeshRenderData> SkelMeshRenderData;
    
    /** 계층구조, Bone Info 및 Transform 배열 저장 */
    FReferenceSkeleton RefSkeleton;

private:
    // TODO: Skeletal 전용 Material이 나온다면 변경 필요 
    TArray<FStaticMaterial*> Materials;
};

