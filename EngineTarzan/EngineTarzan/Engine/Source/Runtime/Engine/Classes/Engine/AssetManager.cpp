#include "AssetManager.h"
#include "Engine.h"

#include <filesystem>
#include "Engine/FObjLoader.h"
#include "UnrealEd/FbxImporter.h"

namespace fs = std::filesystem;

void FAssetRegistry::ScanDirectory(const FString& InRootDir)
{
    AssetMetaDatas.Empty();

    std::string RootAnsi = GetData(InRootDir);
    for (auto& Entry : fs::recursive_directory_iterator(RootAnsi))
    {
        if (!Entry.is_regular_file())
            continue;

        // 전체 경로, 상대 경로(Unix 스타일)
        FString FullPathAnsi = Entry.path().string();
        FString RelPathAnsi = fs::relative(Entry.path(), RootAnsi).generic_string();

        // FString 변환
        FString Id        = RelPathAnsi;             // ex: "Meshes/Hero.fbx"
        FString FullPath  = FullPathAnsi;           // ex: "C:/Project/Content/Meshes/Hero.fbx"
        
        // AssetName: 확장자 제외한 파일명
        FString AssetName    = FPath::GetBaseFilename(Id);                 // ex: "Hero"
        // PackagePath: "/Game/" + 폴더 경로 (언리얼 패키지 룩)
        // AssetType: 확장자로부터 결정 (enum 커스터마이징 필요)
        FString ExtWithDot   = FPath::GetExtension(Id, true).ToLower();    // ex: ".fbx"
        EAssetType AssetType = EAssetType::MAX;
        if (ExtWithDot == TEXT(".fbx"))          AssetType = EAssetType::SkeletalMesh;
        else if (ExtWithDot == TEXT(".png"))     AssetType = EAssetType::Texture2D;
        // ...필요시 확장자 매핑 추가...

        // 파일 크기 (바이트 단위)
        uint32 Size = static_cast<uint32>(fs::file_size(Entry.path()));

        // 메타데이터 구성
        FAssetMetaData Meta;
        Meta.AssetName   = AssetName;
        Meta.FullPath = FPath(FullPath).Normalize();
        Meta.AssetType   = AssetType;
        Meta.Size        = Size;

        AssetMetaDatas.Emplace(Id, Meta);
    }
}

FPath FAssetRegistry::GetPath(const FString& id)
{
    return AssetMetaDatas.Find(id)->FullPath;
}

UAssetManager::UAssetManager()
{
}

bool UAssetManager::IsInitialized()
{
    return GEngine && GEngine->AssetManager;
}

UAssetManager& UAssetManager::Get()
{
    if (UAssetManager* Singleton = GEngine->AssetManager)
    {
        return *Singleton;
    }
    else
    {
        UE_LOG(LogLevel::Error, "Cannot use AssetManager if no AssetManagerClassName is defined!");
        assert(0);
        return *new UAssetManager; // never calls this
    }
}

UAssetManager* UAssetManager::GetIfInitialized()
{
    return GEngine ? GEngine->AssetManager : nullptr;
}

void UAssetManager::Initialize()
{
    AssetRegistry = FAssetRegistry();
    AssetRegistry.ScanDirectory(ContentDirectory.ToString());
    
    // 2) Importer 등록
    //    .fbx → USkeletalMesh 처리기
    static FFbxImporter* SkeletalFbxImp = new FFbxImporter();
    FString Ext = TEXT(".fbx");
    RegisterImporter<USkeletalMeshAsset>(Ext, SkeletalFbxImp);

    // //    .fbx → UStaticMesh 처리기 (예제용)
    // static FFbxStaticMeshImporter* StaticFbxImp = new FFbxStaticMeshImporter();
    // RegisterImporter<UStaticMesh>(TEXT(".fbx"), StaticFbxImp);
    //
    // //    .obj → UStaticMesh 처리기
    // static FObjImporter* ObjImp = new FObjImporter();
    // RegisterImporter<UStaticMesh>(TEXT(".obj"), ObjImp);
    //
    // //    .png → UTexture2D 처리기
    // static FPngImporter* PngImp = new FPngImporter();
    // RegisterImporter<UTexture2D>(TEXT(".png"), PngImp);
}

const TMap<FString, FAssetMetaData>& UAssetManager::GetAssetMetaDatas()
{
    return AssetRegistry.AssetMetaDatas;
}
