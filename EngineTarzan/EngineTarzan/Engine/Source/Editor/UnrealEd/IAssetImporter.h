#pragma once
#include "Engine/SkeletalMesh.h"

struct FAssetData;
class FString;

// T는 UObject 기반 Asset 클래스 (e.g. USkeletalMesh)
template<typename T>
class IAssetImporter
{
public:
    virtual ~IAssetImporter() = default;
    // 파일 경로로부터 Asset 인스턴스 생성 (Import 단계)
    virtual T* Import(const FString& InPath) = 0;

    // 런타임: AssetData → 실제 UObject(T) 인스턴스 생성
    virtual T* CreateAsset(std::shared_ptr<FAssetData> InData) = 0;

    virtual std::shared_ptr<FAssetData> ImportData(const FString& InPath) = 0;
};
