#pragma once
#include "Engine.h"
#include "Actors/Player.h"

/*
    Editor 모드에서 사용될 엔진.
    UEngine을 상속받아 Editor에서 사용 될 기능 구현.
    내부적으로 PIE, Editor World 두 가지 형태로 관리.
*/

DECLARE_MULTICAST_DELEGATE(FOnOpenSkeletalMeshViewerDelegate);
DECLARE_MULTICAST_DELEGATE(FOnCloseSkeletalMeshViewerDelegate);

class AActor;
class USceneComponent;

class UEditorEngine : public UEngine
{
    DECLARE_CLASS(UEditorEngine, UEngine)

public:
    UEditorEngine() = default;

    virtual void Init() override;
    virtual void Tick(float DeltaTime) override;
    void Release() override;

    UWorld* PIEWorld = nullptr;
    UWorld* EditorWorld = nullptr;
    UWorld* StaticMeshViewerWorld = nullptr;

    void StartPIE();
    void BindEssentialObjects();
    void EndPIE();

    void OpenSkeletalMeshViewer();
    void CloseSkeletalMeshViewer();

    // 주석은 UE에서 사용하던 매개변수.
    FWorldContext& GetEditorWorldContext(/*bool bEnsureIsGWorld = false*/);
    FWorldContext* GetPIEWorldContext(/*int32 WorldPIEInstance = 0*/);

public:
    void SelectActor(AActor* InActor);

    // 전달된 액터가 선택된 컴포넌트와 같다면 해제 
    void DeselectActor(AActor* InActor);
    void ClearActorSelection(); 
    
    bool CanSelectActor(const AActor* InActor) const;
    AActor* GetSelectedActor() const;

    void HoverActor(AActor* InActor);

    
    void NewLevel();

    void SelectComponent(USceneComponent* InComponent) const;
    
    // 전달된 InComponent가 현재 선택된 컴포넌트와 같다면 선택 해제
    void DeselectComponent(USceneComponent* InComponent);
    void ClearComponentSelection(); 
    
    bool CanSelectComponent(const USceneComponent* InComponent) const;
    USceneComponent* GetSelectedComponent() const;

    void HoverComponent(USceneComponent* InComponent);

public:
    UEditorPlayer* GetEditorPlayer() const;

private:
    UEditorPlayer* EditorPlayer = nullptr;
};



