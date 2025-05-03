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
    USkeletalMesh() = default;


    void SetImportedModel(FSkeletalMeshLODModel* InModel);

    FSkeletalMeshRenderData* GetSkeletalMeshRenderData() const;


private:

    /** RawData 저장용 Imported Model [Unreal에선 LODModel 배열로 관리] */
    FSkeletalMeshLODModel* ImportedModel;

    FReferenceSkeleton RefSkeleton;

    /** 런타임용 렌더링 리소스 */
     FSkeletalMeshRenderData* SkeletalMeshRenderData;
};

