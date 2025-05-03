#include "ReferenceSkeleton.h"

FArchive& operator<<(FArchive& Ar, FMeshBoneInfo& F)
{
    Ar << F.Name << F.ParentIndex;
    return Ar;
}
