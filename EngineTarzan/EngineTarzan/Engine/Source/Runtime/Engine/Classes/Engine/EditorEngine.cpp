#include "EditorEngine.h"

#include "World/World.h"
#include "Level.h"
#include "Actors/Cube.h"
#include "Actors/DirectionalLightActor.h"
#include "Actors/SkeletalActor.h"
#include "BaseGizmos/TransformGizmo.h"
#include "GameFramework/Actor.h"
#include "Classes/Engine/AssetManager.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/SkeletalMeshViewportClient.h"
#include "UObject/UObjectIterator.h"

class ASkeletalActor;

namespace PrivateEditorSelection
{
    static AActor* GActorSelected = nullptr;
    static AActor* GActorHovered = nullptr;

    static USceneComponent* GComponentSelected = nullptr;
    static USceneComponent* GComponentHovered = nullptr;
}

void UEditorEngine::Init()
{
    Super::Init();

    // Initialize the engine
    GEngine = this;

    FWorldContext& EditorWorldContext = CreateNewWorldContext(EWorldType::Editor);

    EditorWorld = UWorld::CreateWorld(this, EWorldType::Editor, FString("EditorWorld"));

    EditorWorldContext.SetCurrentWorld(EditorWorld);
    ActiveWorld = EditorWorld;

    EditorPlayer = FObjectFactory::ConstructObject<UEditorPlayer>(this);
    EditorPlayer->Initialize();

    if (AssetManager == nullptr)
    {
        AssetManager = FObjectFactory::ConstructObject<UAssetManager>(this);
        assert(AssetManager);
        AssetManager->InitAssetManager();
    }
    LoadLevel("Saved/AutoSaves.scene");
}

void UEditorEngine::Release()
{
    SaveLevel("Saved/AutoSaves.scene");
}

void UEditorEngine::Tick(float DeltaTime)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::PIE)
        {
            if (UWorld* World = WorldContext->World())
            {
                World->Tick(DeltaTime);
                ULevel* Level = World->GetActiveLevel();
                TArray CachedActors = Level->Actors;
                if (Level)
                {
                    for (AActor* Actor : CachedActors)
                    {
                        if (Actor)
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
    }
}

void UEditorEngine::StartPIE()
{
    if (PIEWorld)
    {
        UE_LOG(LogLevel::Warning, TEXT("PIEWorld already exists!"));
        return;
    }
    this->ClearActorSelection(); // Editor World 기준 Select Actor 해제 
    
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    Handler->OnPIEModeStart();

    FWorldContext& PIEWorldContext = CreateNewWorldContext(EWorldType::PIE);

    PIEWorld = Cast<UWorld>(EditorWorld->Duplicate(this));
    PIEWorld->WorldType = EWorldType::PIE;

    PIEWorldContext.SetCurrentWorld(PIEWorld);
    ActiveWorld = PIEWorld;
    
    BindEssentialObjects();
    
    PIEWorld->BeginPlay();
    // 여기서 Actor들의 BeginPlay를 해줄지 안에서 해줄 지 고민.
    // WorldList.Add(GetWorldContextFromWorld(PIEWorld));
}

void UEditorEngine::BindEssentialObjects()
{
    // TODO: 플레이어 컨트롤러가 먼저 만들어져야 함.
    //실수로 안만들면 넣어주기
    if (ActiveWorld->GetMainPlayer() == nullptr)
    {
        APlayer* TempPlayer = ActiveWorld->SpawnActor<APlayer>();
        TempPlayer->SetActorLabel(TEXT("OBJ_PLAYER"));
        TempPlayer->SetActorTickInEditor(false);
        ActiveWorld->SetMainPlayer(TempPlayer);
    }
    
    //마찬가지
    for (const auto iter: TObjectRange<APlayer>())
    {
        if (iter->GetWorld() == ActiveWorld)
        {
            ActiveWorld->SetMainPlayer(iter);
            break;
        }
    }
    
    //무조건 PIE들어갈때 만들어주기
    APlayerController* PlayerController = ActiveWorld->SpawnActor<APlayerController>();
    PlayerController->SetActorLabel(TEXT("OBJ_PLAYER_CONTROLLER"));
    PlayerController->SetActorTickInEditor(false);
    ActiveWorld->SetPlayerController(PlayerController);

    
    ActiveWorld->GetPlayerController()->Possess(ActiveWorld->GetMainPlayer());
}

void UEditorEngine::EndPIE()
{
    if (PIEWorld)
    {
        this->ClearActorSelection(); // PIE World 기준 Select Actor 해제 
        //WorldList.Remove(*GetWorldContextFromWorld(PIEWorld.get()));
        WorldList.Remove(GetWorldContextFromWorld(PIEWorld));
        PIEWorld->Release();
        GUObjectArray.MarkRemoveObject(PIEWorld);
        PIEWorld = nullptr;

        // TODO: PIE에서 EditorWorld로 돌아올 때, 기존 선택된 Picking이 유지되어야 함. 현재는 에러를 막기위해 임시조치.
        DeselectActor(GetSelectedActor());
        DeselectComponent(GetSelectedComponent());
    }

    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    Handler->OnPIEModeEnd();
    // 다시 EditorWorld로 돌아옴.
    ActiveWorld = EditorWorld;
}

void UEditorEngine::OpenSkeletalMeshViewer()
{
    if (StaticMeshViewerWorld != nullptr)
    {
        UE_LOG(LogLevel::Warning, TEXT("StaticMeshViewerWorld already exists!"));
        return;
    }

    this->ClearActorSelection(); // Editor World 기준 Select Actor 해제 
    
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    StaticMeshViewerWorld = UWorld::CreateWorld(this, EWorldType::SkeletalMeshViewer, FString("StaticMeshViwerWorld"));
    StaticMeshViewerWorld->WorldType = EWorldType::SkeletalMeshViewer;
    
    ActiveWorld = StaticMeshViewerWorld;

    ASkeletalActor* actor = ActiveWorld->SpawnActor<ASkeletalActor>();
    actor->SetActorLocation(FVector(0, 0, 0));
    actor->SetActorRotation(FRotator(0, 0, 0));
    
    ADirectionalLight* dirLight = ActiveWorld->SpawnActor<ADirectionalLight>();
    dirLight->SetActorRotation(FRotator(30, 0, 0));
    
    FWorldContext& ViwerWorldContext = CreateNewWorldContext(EWorldType::SkeletalMeshViewer);
    
    ViwerWorldContext.SetCurrentWorld(ActiveWorld);
    Handler->OnSkeletalMeshViewerStartDelegate.Broadcast();

    const SLevelEditor* LevelEd = GEngineLoop.GetLevelEditor();
    LevelEd->GetSkeletalMeshViewportClient()->SetSkeletalActor(actor);
}

void UEditorEngine::CloseSkeletalMeshViewer()
{
    if (StaticMeshViewerWorld)
    {
        this->ClearActorSelection(); // StaticMeshViewerWorld 기준 Select Actor 해제 
        WorldList.Remove(GetWorldContextFromWorld(StaticMeshViewerWorld));
        StaticMeshViewerWorld->Release();
        GUObjectArray.MarkRemoveObject(StaticMeshViewerWorld);
        StaticMeshViewerWorld = nullptr;

        // TODO: PIE에서 EditorWorld로 돌아올 때, 기존 선택된 Picking이 유지되어야 함. 현재는 에러를 막기위해 임시조치.
        DeselectActor(GetSelectedActor());
        DeselectComponent(GetSelectedComponent());
    }

    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
    
    // 다시 EditorWorld로 돌아옴.
    ActiveWorld = EditorWorld;
    Handler->OnSkeletalMeshViewerEndDelegate.Broadcast();
}

FWorldContext& UEditorEngine::GetEditorWorldContext(/*bool bEnsureIsGWorld*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Editor)
        {
            return *WorldContext;
        }
    }
    return CreateNewWorldContext(EWorldType::Editor);
}

FWorldContext* UEditorEngine::GetPIEWorldContext(/*int32 WorldPIEInstance*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::PIE)
        {
            return WorldContext;
        }
    }
    return nullptr;
}

void UEditorEngine::SelectActor(AActor* InActor)
{
    if (InActor && CanSelectActor(InActor))
    {
        PrivateEditorSelection::GActorSelected = InActor;
    }
}

void UEditorEngine::DeselectActor(AActor* InActor)
{
    if (PrivateEditorSelection::GActorSelected == InActor && InActor)
    {
        PrivateEditorSelection::GActorSelected = nullptr;
        ClearComponentSelection();
    }
}

void UEditorEngine::ClearActorSelection()
{
    PrivateEditorSelection::GActorSelected = nullptr;
}

bool UEditorEngine::CanSelectActor(const AActor* InActor) const
{
    return InActor != nullptr && InActor->GetWorld() == ActiveWorld && !InActor->IsActorBeingDestroyed();
}

AActor* UEditorEngine::GetSelectedActor() const
{
    return PrivateEditorSelection::GActorSelected;
}

void UEditorEngine::HoverActor(AActor* InActor)
{
    if (InActor)
    {
        PrivateEditorSelection::GActorHovered = InActor;
    }
}

void UEditorEngine::NewLevel()
{
    ClearActorSelection();
    ClearComponentSelection();

    if (ActiveWorld->GetActiveLevel())
    {
        ActiveWorld->GetActiveLevel()->Release();
    }
}

void UEditorEngine::SelectComponent(USceneComponent* InComponent) const
{
    if (!InComponent || !CanSelectComponent(InComponent))
    {
        return;
    }

    // 현재 선택 컴포넌트 저장
    PrivateEditorSelection::GComponentSelected = InComponent;

    // 로컬 좌표 모드인지 체크
    const bool bUseLocal = (EditorPlayer->GetCoordMode() == ECoordMode::CDM_LOCAL) || (EditorPlayer->GetControlMode() == EControlMode::CM_SCALE);

    // gizmo 위치·회전 갱신 람다
    auto UpdateActor = [&](AActor* Gizmo)
    {
        if (!Gizmo) return;
        Gizmo->SetActorLocation(InComponent->GetWorldLocation());
        Gizmo->SetActorRotation(
            bUseLocal 
                ? InComponent->GetWorldRotation() 
                : FRotator::ZeroRotator
        );
    };

    // 여러 뷰포트 지원
    SLevelEditor* LevelEditor = GEngineLoop.GetLevelEditor();
    if (LevelEditor->IsMultiViewport())
    {
        auto* Viewports = LevelEditor->GetViewports();
        constexpr size_t NumViewports = sizeof(Viewports) / sizeof(Viewports[0]);
        for (int i = 0; i < NumViewports; ++i)
        {
            if (const std::shared_ptr<FEditorViewportClient> EditorViewportClient = Viewports[i])
            {
                UpdateActor(EditorViewportClient->GetGizmoActor());
            }
        }
    }
    else
    {
        if (const std::shared_ptr<FEditorViewportClient> EditorViewportClient = LevelEditor->GetActiveViewportClient())
        {
            UpdateActor(EditorViewportClient->GetGizmoActor());
        }
    } 
}

void UEditorEngine::DeselectComponent(USceneComponent* InComponent)
{
    // 전달된 InComponent가 현재 선택된 컴포넌트와 같다면 선택 해제
    if (PrivateEditorSelection::GComponentSelected == InComponent && InComponent != nullptr)
    {
        PrivateEditorSelection::GComponentSelected = nullptr;
    }
}

void UEditorEngine::ClearComponentSelection()
{
    PrivateEditorSelection::GComponentSelected = nullptr;
}

bool UEditorEngine::CanSelectComponent(const USceneComponent* InComponent) const
{
    return InComponent != nullptr && InComponent->GetOwner() && InComponent->GetOwner()->GetWorld() == ActiveWorld && !InComponent->GetOwner()->IsActorBeingDestroyed();
}

USceneComponent* UEditorEngine::GetSelectedComponent() const
{
    return PrivateEditorSelection::GComponentSelected;
}

void UEditorEngine::HoverComponent(USceneComponent* InComponent)
{
    if (InComponent)
    {
        PrivateEditorSelection::GComponentHovered = InComponent;
    }
}

UEditorPlayer* UEditorEngine::GetEditorPlayer() const
{
    return EditorPlayer;
}
