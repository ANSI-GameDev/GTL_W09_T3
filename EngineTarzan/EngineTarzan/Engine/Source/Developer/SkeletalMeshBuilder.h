// SkeletalMeshBuilder.h
#pragma once

class FSkeletalMeshLODModel;
struct FSkeletalMeshRenderData;

class FSkeletalMeshBuilder
{
public:
    /** LODModel → RenderData 변환 */
    static void BuildRenderData(
        const FSkeletalMeshLODModel& InLODModel,
        FSkeletalMeshRenderData& OutRenderData);
    static void ConvertLODModelToRenderData(const FSkeletalMeshLODModel& LODModel, FSkeletalMeshRenderData& OutRenderData);
};
