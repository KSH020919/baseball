#include "MyGameModeBase.h"
#include "MyGameStateBase.h"
#include "MyPlayerController.h"
#include "MyPlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"

AMyGameModeBase::AMyGameModeBase()
{
    // 우리가 만든 클래스들을 게임에서 사용하도록 지정합니다.
    PlayerControllerClass = AMyPlayerController::StaticClass();
    PlayerStateClass = AMyPlayerState::StaticClass();
    GameStateClass = AMyGameStateBase::StaticClass();

    // 이번 게임에는 이동 캐릭터가 필요하지 않습니다.
    DefaultPawnClass = nullptr;
    bStartPlayersAsSpectators = true;
}

void AMyGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    // 사람이 부족하면 대기 상태가 됩니다.
    ResetGame();
}

void AMyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AMyPlayerState* PS =
        NewPlayer->GetPlayerState<AMyPlayerState>();

    if (!PS) return;

    // 접속한 플레이어에게 구분 가능한 이름을 붙입니다.
    PS->SetPlayerName(
        FString::Printf(
            TEXT("Player %d"),
            NextPlayerNumber++
        )
    );

    // 진행 중에 접속하면 다음 라운드까지 기다립니다.
    PS->ResetForRound(false);

    // 서버가 접속한 플레이어를 기억합니다.
    Players.Add(PS);

    AMyGameStateBase* GS =
        GetGameState<AMyGameStateBase>();

    // 대기 중이라면 인원이 충분한지 다시 확인합니다.
    if (GS && GS->Phase == EBaseballPhase::Waiting)
    {
        ResetGame();
    }
}

void AMyGameModeBase::Logout(AController* Exiting)
{
    const AMyPlayerState* Leaving =
        Exiting->GetPlayerState<AMyPlayerState>();

    // 사라진 플레이어와 이번에 퇴장하는 플레이어를 제외합니다.
    Players.RemoveAll(
        [Leaving](const TWeakObjectPtr<AMyPlayerState>& Player)
        {
            return !Player.IsValid() || Player.Get() == Leaving;
        }
    );

    Super::Logout(Exiting);

    // 퇴장으로 종료 조건이 충족됐는지 다시 확인합니다.
    CheckDraw();
}

void AMyGameModeBase::GenerateRandomNumbers()
{
    // 이전 정답을 지웁니다.
    SecretNumber.Empty();

    TArray<int32> Pool =
    {
        1, 2, 3, 4, 5, 6, 7, 8, 9
    };

    for (int32 Count = 0; Count < 3; ++Count)
    {
        // 뽑을 위치를 정합니다.
        const int32 Index =
            FMath::RandRange(0, Pool.Num() - 1);

        // 뽑은 숫자를 문자열로 붙입니다.
        SecretNumber += FString::FromInt(Pool[Index]);

        // 같은 숫자가 다시 뽑히지 않게 제거합니다.
        Pool.RemoveAt(Index);
    }

#if !UE_BUILD_SHIPPING
    // 개발 중에만 서버 로그에 정답을 표시합니다.
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Server] Answer: %s"),
        *SecretNumber
    );
#endif
}

bool AMyGameModeBase::IsValidInput(
    const FString& Input
) const
{
    // 정확히 3글자여야 합니다.
    if (Input.Len() != 3) return false;

    for (int32 Index = 0; Index < 3; ++Index)
    {
        // 각 글자는 1~9여야 합니다.
        if (Input[Index] < TEXT('1') ||
            Input[Index] > TEXT('9'))
        {
            return false;
        }

        // 앞에 같은 숫자가 있었다면 중복입니다.
        for (int32 Previous = 0; Previous < Index; ++Previous)
        {
            if (Input[Index] == Input[Previous])
            {
                return false;
            }
        }
    }

    // 모든 검사를 통과했습니다.
    return true;
}

FString AMyGameModeBase::CheckAnswer(
    const FString& Input,
    int32& Strikes
) const
{
    // SubmitGuess에서 검사를 통과한 입력만 이 함수에 전달합니다.
    Strikes = 0;
    int32 Balls = 0;

    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (Input[Index] == SecretNumber[Index])
        {
            // 숫자와 위치가 모두 같습니다.
            ++Strikes;
        }
        else if (
            SecretNumber.Contains(
                FString::Chr(Input[Index])
            )
            )
        {
            // 같은 위치는 아니지만 정답에 해당 숫자가 있습니다.
            ++Balls;
        }
    }

    if (Strikes == 0 && Balls == 0)
    {
        return TEXT("OUT");
    }

    return FString::Printf(
        TEXT("%dS%dB"),
        Strikes,
        Balls
    );
}

void AMyGameModeBase::ResetGame()
{
    AMyGameStateBase* GS =
        GetGameState<AMyGameStateBase>();

    // 이미 진행 중인 라운드는 초기화하지 않습니다.
    if (!GS || GS->Phase == EBaseballPhase::Playing)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(ResetTimer);

    // 더 이상 존재하지 않는 플레이어를 정리합니다.
    Players.RemoveAll(
        [](const TWeakObjectPtr<AMyPlayerState>& Player)
        {
            return !Player.IsValid();
        }
    );

    // 두 명 이상이어야 시작합니다.
    const bool bCanStart = Players.Num() >= 2;

    // 모든 플레이어의 횟수와 참가 상태를 초기화합니다.
    for (const auto& Player : Players)
    {
        Player->ResetForRound(bCanStart);
    }

    SecretNumber.Empty();

    GS->Phase = bCanStart
        ? EBaseballPhase::Playing
        : EBaseballPhase::Waiting;

    GS->Announcement =
        TEXT("Waiting for at least 2 players.");

    if (bCanStart)
    {
        // 새 정답을 서버에서 만듭니다.
        GenerateRandomNumbers();

        ++GS->RoundNumber;

        GS->Announcement = FString::Printf(
            TEXT("ROUND %d | Guess 3 different digits from 1 to 9."),
            GS->RoundNumber
        );

        GS->MulticastMessage(
            TEXT("[Round] A new round has started. You have 3 attempts.")
        );
    }

    // 최신 공지와 라운드 정보를 전송합니다.
    GS->ForceNetUpdate();
}

void AMyGameModeBase::SubmitGuess(
    AMyPlayerController* Sender,
    const FString& Input,
    int32 RoundNumber
)
{
    AMyGameStateBase* GS =
        GetGameState<AMyGameStateBase>();

    AMyPlayerState* PS = Sender
        ? Sender->GetPlayerState<AMyPlayerState>()
        : nullptr;

    if (!GS || !PS) return;

    // 클라이언트가 버튼을 우회해도 서버에서 다시 검사합니다.
    if (GS->Phase != EBaseballPhase::Playing ||
        !PS->IsParticipating())
    {
        Sender->ClientNotice(
            TEXT("Please wait for your next round.")
        );

        return;
    }

    // 이전 라운드에서 늦게 도착한 요청은 거부합니다.
    if (RoundNumber != GS->RoundNumber)
    {
        Sender->ClientNotice(
            TEXT("The round changed. Please try again.")
        );

        return;
    }

    // 이미 기회를 모두 사용했으면 더 제출할 수 없습니다.
    if (!PS->HasAttemptsRemaining())
    {
        Sender->ClientNotice(
            TEXT("You have used all 3 attempts.")
        );

        return;
    }

    // 잘못된 입력에는 기회를 차감하지 않습니다.
    if (!IsValidInput(Input))
    {
        Sender->ClientNotice(
            TEXT("Enter 3 different digits from 1 to 9. No attempt was used.")
        );

        return;
    }

    // 모든 검사를 통과한 뒤에만 기회를 차감합니다.
    if (!PS->ConsumeAttempt()) return;

    int32 Strikes = 0;

    // 결과 문자열과 스트라이크 개수를 얻습니다.
    const FString Result =
        CheckAnswer(Input, Strikes);

    // 모든 플레이어에게 제출 결과를 알립니다.
    GS->MulticastMessage(
        FString::Printf(
            TEXT("%s: %s -> %s [%d / %d]"),
            *PS->GetPlayerName(),
            *Input,
            *Result,
            PS->GetCurrentAttemptCount(),
            PS->GetMaxAttemptCount()
        )
    );

    // 세 번째 시도의 정답도 승리이므로 무승부보다 먼저 검사합니다.
    if (Strikes == 3)
    {
        FinishRound(
            FString::Printf(
                TEXT("HOME RUN! %s wins!"),
                *PS->GetPlayerName()
            )
        );

        return;
    }

    // 정답자가 없다면 모든 참가자의 남은 기회를 확인합니다.
    CheckDraw();
}

void AMyGameModeBase::SubmitChat(
    AMyPlayerController* Sender,
    const FString& Input
)
{
    AMyGameStateBase* GS =
        GetGameState<AMyGameStateBase>();

    AMyPlayerState* PS = Sender
        ? Sender->GetPlayerState<AMyPlayerState>()
        : nullptr;

    if (!GS || !PS) return;

    // 긴 채팅은 서버에서 제한합니다.
    if (Input.Len() > 200)
    {
        Sender->ClientNotice(
            TEXT("Chat messages must be 200 characters or fewer.")
        );

        return;
    }

    // 앞뒤 공백을 제거합니다.
    FString Message = Input.TrimStartAndEnd();

    // 줄 바꿈과 탭은 공백으로 바꿉니다.
    Message.ReplaceInline(TEXT("\r"), TEXT(" "));
    Message.ReplaceInline(TEXT("\n"), TEXT(" "));
    Message.ReplaceInline(TEXT("\t"), TEXT(" "));

    if (Message.IsEmpty()) return;

    GS->MulticastMessage(
        FString::Printf(
            TEXT("[Chat] %s: %s"),
            *PS->GetPlayerName(),
            *Message
        )
    );

    // 일반 채팅에서는 ConsumeAttempt()를 호출하지 않습니다.
}

void AMyGameModeBase::CheckDraw()
{
    const AMyGameStateBase* GS =
        GetGameState<AMyGameStateBase>();

    if (!GS || GS->Phase != EBaseballPhase::Playing)
    {
        return;
    }

    int32 Participants = 0;

    for (const auto& Player : Players)
    {
        // 사라진 플레이어와 다음 라운드 대기자는 제외합니다.
        if (!Player.IsValid() || !Player->IsParticipating())
        {
            continue;
        }

        ++Participants;

        // 한 명이라도 기회가 남으면 계속 진행합니다.
        if (Player->HasAttemptsRemaining())
        {
            return;
        }
    }

    // 모두 퇴장한 경우와 모두 기회를 사용한 경우를 구분합니다.
    FinishRound(
        Participants == 0
        ? TEXT("All participants left.")
        : TEXT("DRAW! Everyone used all attempts.")
    );
}

void AMyGameModeBase::FinishRound(
    const FString& Message
)
{
    AMyGameStateBase* GS =
        GetGameState<AMyGameStateBase>();

    if (!GS || GS->Phase != EBaseballPhase::Playing)
    {
        return;
    }

    // 추가 정답 제출을 막습니다.
    GS->Phase = EBaseballPhase::Finished;

    // 공지 영역에 결과를 유지합니다.
    GS->Announcement =
        Message +
        TEXT(" Next round in 5 seconds if 2 players are available.");

    GS->ForceNetUpdate();

    // 채팅 기록에도 결과를 남깁니다.
    GS->MulticastMessage(
        TEXT("[Result] ") + Message
    );

    // 5초 뒤 한 번만 ResetGame을 실행합니다.
    GetWorldTimerManager().SetTimer(
        ResetTimer,
        this,
        &AMyGameModeBase::ResetGame,
        5.0f,
        false
    );
}