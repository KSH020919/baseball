#include "MyPlayerController.h"
#include "MyGameModeBase.h"
#include "MyGameStateBase.h"
#include "MyUserWidget.h"
#include "Engine/World.h"
#include "TimerManager.h"

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 본인이 조작하는 클라이언트에서만 UI를 만듭니다.
    if (!IsLocalController() || GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    GameWidget = CreateWidget<UMyUserWidget>(
        this,
        UMyUserWidget::StaticClass()
    );

    if (!GameWidget) return;

    // UI가 키보드 입력을 받을 수 있게 합니다.
    GameWidget->SetIsFocusable(true);

    // 생성한 UI를 화면에 붙입니다.
    GameWidget->AddToViewport();

    // 버튼을 누를 수 있도록 마우스를 표시합니다.
    bShowMouseCursor = true;

    // 게임 캐릭터 대신 UI에 입력을 전달합니다.
    FInputModeUIOnly Mode;
    Mode.SetWidgetToFocus(GameWidget->TakeWidget());
    Mode.SetLockMouseToViewportBehavior(
        EMouseLockMode::DoNotLock
    );

    SetInputMode(Mode);

    // UI가 준비되기 전에 받은 메시지도 표시합니다.
    for (const FString& Message : PendingMessages)
    {
        GameWidget->AppendMessage(Message);
    }

    PendingMessages.Reset();

    // 첫 화면을 갱신합니다.
    RefreshUI();

    // 이후에는 0.1초마다 최신 상태를 표시합니다.
    GetWorldTimerManager().SetTimer(
        UITimer,
        this,
        &AMyPlayerController::RefreshUI,
        0.1f,
        true
    );
}

void AMyPlayerController::EndPlay(
    const EEndPlayReason::Type Reason
)
{
    // 종료된 화면을 계속 갱신하지 않도록 타이머를 정리합니다.
    GetWorldTimerManager().ClearTimer(UITimer);

    if (GameWidget)
    {
        GameWidget->RemoveFromParent();
    }

    GameWidget = nullptr;

    Super::EndPlay(Reason);
}

void AMyPlayerController::RefreshUI()
{
    // UI가 GameState와 PlayerState를 읽어서 화면을 갱신합니다.
    if (GameWidget)
    {
        GameWidget->RefreshState();
    }
}

void AMyPlayerController::SendGuess(const FString& Input)
{
    const AMyGameStateBase* GS =
        GetWorld()->GetGameState<AMyGameStateBase>();

    // 입력값과 현재 라운드 번호를 함께 서버로 보냅니다.
    if (IsLocalController() && GS)
    {
        ServerSubmitGuess(Input, GS->RoundNumber);
    }
}

void AMyPlayerController::SendChat(const FString& Input)
{
    // 일반 채팅도 서버를 거쳐서 전달합니다.
    if (IsLocalController())
    {
        ServerSubmitChat(Input);
    }
}

void AMyPlayerController::ReceiveMessage(
    const FString& Message
)
{
    if (!IsLocalController()) return;

    if (GameWidget)
    {
        // UI가 있으면 바로 표시합니다.
        GameWidget->AppendMessage(Message);
    }
    else
    {
        // UI가 없으면 나중에 표시하도록 저장합니다.
        PendingMessages.Add(Message);

        if (PendingMessages.Num() > 100)
        {
            PendingMessages.RemoveAt(0);
        }
    }
}

void AMyPlayerController::ClientNotice_Implementation(
    const FString& Message
)
{
    // 개인 안내는 해당 플레이어에게만 보입니다.
    ReceiveMessage(TEXT("[Notice] ") + Message);
}

void AMyPlayerController::ServerSubmitGuess_Implementation(
    const FString& Input,
    int32 RoundNumber
)
{
    // 이 함수의 본문은 서버에서 실행됩니다.
    const double Now = GetWorld()->GetTimeSeconds();

    // 너무 빠르게 반복된 제출은 무시합니다.
    if (Now - LastGuessTime < 0.2) return;

    LastGuessTime = Now;

    AMyGameModeBase* GM =
        GetWorld()->GetAuthGameMode<AMyGameModeBase>();

    // 실제 판정은 GameMode에 맡깁니다.
    if (GM)
    {
        GM->SubmitGuess(this, Input, RoundNumber);
    }
}

void AMyPlayerController::ServerSubmitChat_Implementation(
    const FString& Input
)
{
    const double Now = GetWorld()->GetTimeSeconds();

    // 채팅도 지나친 반복 전송을 제한합니다.
    if (Now - LastChatTime < 0.3) return;

    LastChatTime = Now;

    AMyGameModeBase* GM =
        GetWorld()->GetAuthGameMode<AMyGameModeBase>();

    if (GM)
    {
        GM->SubmitChat(this, Input);
    }
}