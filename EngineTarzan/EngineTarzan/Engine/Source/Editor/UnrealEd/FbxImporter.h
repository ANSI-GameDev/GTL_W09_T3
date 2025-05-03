#pragma once
#include <fbxsdk.h>

#include "HAL/PlatformType.h"

struct FReferenceSkeleton;
class FString;
class USkeletalMesh;

class FFbxImporter
{

public:

    static bool ParseReferenceSkeleton(const FString& InFilePath, FReferenceSkeleton& OutRefSkeleton);

    static USkeletalMesh* ImportSkeletalMesh(const FString& InFilePath);

private:
    // FBX 노드 재귀 순회하여 Skeleton 노드만 RefSkeleton에 추가
    static void BuildReferenceSkeleton(FbxNode* Node, FReferenceSkeleton& OutRefSkeleton, uint32 ParentIndex, int32 Depth);
};

