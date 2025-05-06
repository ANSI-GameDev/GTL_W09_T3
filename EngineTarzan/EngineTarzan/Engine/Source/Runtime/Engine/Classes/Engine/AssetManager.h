#pragma once
#include "Asset/UAsset.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Container/Path.h"
#include "UnrealEd/IAssetImporter.h"
#include "UObject/Casts.h"

class UAsset;

struct FAssetRegistry
{
    // Content/ 폴더 전체를 스캔해 id→path 매핑을 채운다
    void ScanDirectory(const FString& InRootDir);

    // id에 해당하는 파일 경로 반환 (없으면 빈 문자열)
    FPath GetPath(const FString& id);
    
    // key: Content/ 이하 상대 경로, value: 실제 파일 시스템 상 경로
    TMap<FString, FAssetMetaData> AssetMetaDatas;
};

class UAssetManager : public UObject
{
    DECLARE_CLASS(UAssetManager, UObject)

private:
    FAssetRegistry AssetRegistry;

public:
    UAssetManager();

    template<typename T>
    void RegisterImporter(FString& ext, IAssetImporter<T>* InAssetImporter)
    {
        GetImporterMap<T>()[ext] = InAssetImporter;
    }

    static bool IsInitialized();

    /** UAssetManager를 가져옵니다. */
    static UAssetManager& Get();

    /** UAssetManager가 존재하면 가져오고, 없으면 nullptr를 반환합니다. */
    static UAssetManager* GetIfInitialized();
    
    void Initialize();

    const TMap<FString, FAssetMetaData>& GetAssetMetaDatas();

    template<typename T>
    T* GetAsset(const FString& InAssetName)
    {
        const auto pPtr = LoadedAssets.Find(InAssetName);
        if (pPtr != nullptr)
        {
            UAsset* FoundAsset = *pPtr;
            if (FoundAsset != nullptr)
            {
                return Cast<T>(FoundAsset);
            }
        }

        // 2) 메타데이터 조회
        const FAssetMetaData* Meta = AssetRegistry.AssetMetaDatas.Find(InAssetName);
        if (!Meta)
        {
            return nullptr;
        }

        FPath BinaryPath = Meta->FullPath;
        // TODO : Combine이 "/"으로 붙는 거라 이거로 하면 안됨
        BinaryPath = BinaryPath.Combine(TEXT(".bin"));

        const bool bHasBinary = FPath::Exists(BinaryPath.ToString());
        const FPath& LoadPath = bHasBinary ? BinaryPath : Meta->FullPath;

        // 4) Importer 조회 (확장자 키 사용)
        FString Ext = LoadPath.GetExtension(true).ToLower();
        auto piter = GetImporterMap<T>().Find(Ext);
        if (piter == nullptr)
        {
            UE_LOG(LogLevel::Warning, "[UAssetManager] Importer not found. Id=\"%s\", Ext=\"%s\"", *InAssetName, *Ext);
            return nullptr;
        }

        IAssetImporter<T>* Importer = *piter;

        // 5) Import = ImportData → CreateAsset
        T* Asset = Importer->Import(LoadPath.ToString());
        if (!Asset)
        {
            return nullptr;
        }

        // 6) 캐싱 후 반환
        LoadedAssets.Add(InAssetName, Asset);
        return Asset;
    }

private:

    // 기본 콘텐츠 디렉토리 (파일 시스템 경로)
    FPath ContentDirectory = FPath("Contents/");

    // 로드된 UObject 인스턴스 캐시
    TMap<FName, UAsset*> LoadedAssets; 
    
    // 확장자별 필터 목록
    TArray<FString> ObjExtensions      = { TEXT(".obj") };
    TArray<FString> FbxExtensions      = { TEXT(".fbx") };
    TArray<FString> TextureExtensions  = { TEXT(".png"), TEXT(".dds"), TEXT(".jpg") };
    TArray<FString> MaterialExtensions = { TEXT(".mtl") };

    template<typename T>
    TMap<FString, IAssetImporter<T>*>& GetImporterMap() const
    {
        static TMap<FString, IAssetImporter<T>*> map;
        return map;
    }
};
