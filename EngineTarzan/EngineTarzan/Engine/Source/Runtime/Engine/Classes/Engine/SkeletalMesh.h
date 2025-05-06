#pragma once
#include "ReferenceSkeleton.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"


class FSkeletalMeshLODModel;
class FSkeletalMeshRenderData;

class USkeletalMesh : public UObject
{
    DECLARE_CLASS(USkeletalMesh, UObject)
public:
    USkeletalMesh();
    void Initialize();

    void SetImportedModel(FSkeletalMeshLODModel* InModel);
    void SetSkeletalMeshRenderData(FSkeletalMeshRenderData* InRenderData);
    void SetRefSkeleton(const FReferenceSkeleton& InRefSkeleton);

    FSkeletalMeshRenderData* GetRenderData() const;
    FSkeletalMeshLODModel* GetImportedModel() const;
    const FReferenceSkeleton& GetRefSkeleton() const;

    TArray<FStaticMaterial*>& GetMaterials() { return Materials; }


public:

    /** RawData 저장용 Imported Model [Unreal에선 LODModel 배열로 관리] */
    FSkeletalMeshLODModel* ImportedModel;

    /** 런타임용 렌더링 리소스 */
    FSkeletalMeshRenderData* SkelMeshRenderData;

    /** 계층구조, Bone Info 및 Transform 배열 저장 */
    FReferenceSkeleton RefSkeleton;

private:
    // TODO: Skeletal 전용 Material이 나온다면 변경 필요 
    TArray<FStaticMaterial*> Materials;
};

