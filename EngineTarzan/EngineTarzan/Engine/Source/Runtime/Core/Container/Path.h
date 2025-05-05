#pragma once
#include <filesystem>

#include "Array.h"
#include "String.h"

class FString;

struct FPath
{
    using PathType = std::filesystem::path;

    /** 파일명(확장자 제외) 반환 */
    static FString GetBaseFilename(const FString& InPath);

    /** 확장자 반환 (bIncludeDot=true → ".txt", false → "txt") */
    static FString GetExtension(const FString& InPath, bool bIncludeDot = true);

    /** 디렉토리 경로 반환 (마지막 구분자 제외) */
    static FString GetDirectory(const FString& InPath);

    /** 두 경로를 합침 (중간에 Separator 자동 처리) */
    static FString Combine(const FString& A, const FString& B);

    /** 확장자 변경 (NewExt에 "." 포함 가능) */
    static FString ChangeExtension(const FString& InPath, const FString& NewExt);

    /** 경로 정규화 ("//", "../" 등 제거) */
    static FString Normalize(const FString& InPath);

    /** 절대 경로 반환 */
    static FString GetAbsolutePath(const FString& InPath);

    /** base로부터 to까지의 상대 경로 반환 */
    static FString GetRelativePath(const FString& Base, const FString& To);

    /** 존재 여부 확인 */
    static bool Exists(const FString& InPath);

    /** 디렉토리 여부 확인 */
    static bool IsDirectory(const FString& InPath);

    /** 일반 파일 여부 확인 */
    static bool IsFile(const FString& InPath);

    /** 디렉토리(및 하위폴더) 생성 */
    static bool CreateDirectories(const FString& InPath);

    /** 파일 삭제 */
    static bool RemoveFile(const FString& InPath);

    /** 디렉토리 내 파일/폴더 목록 반환 */
    static TArray<FString> ListDirectory(const FString& InPath, bool bRecursive = false);
};
