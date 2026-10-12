#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

class AMyPlayerController;
class AMyPlayerState;

// 서버에만 존재하며 게임의 정답과 규칙을 관리합니다.
UCLASS()
class HOMERUN_API AMyGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    // 사용할 Controller, PlayerState, GameState를 지정합니다.
    AMyGameModeBase();

    // 플레이어 접속과 퇴장을 처리합니다.
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // 서버의 PlayerController가 요청을 전달하는 함수입니다.
    void SubmitGuess(
        AMyPlayerController* Sender,
        const FString& Input,
        int32 RoundNumber
    );

    void SubmitChat(
        AMyPlayerController* Sender,
        const FString& Input
    );

protected:
    virtual void BeginPlay() override;

private:
    // 1~9 중 서로 다른 숫자 3개를 만듭니다.
    void GenerateRandomNumbers();

    // 길이, 숫자 범위, 중복을 검사합니다.
    bool IsValidInput(const FString& Input) const;

    // 판정 문자열을 반환하고, Strikes에 스트라이크 수를 넣습니다.
    FString CheckAnswer(
        const FString& Input,
        int32& Strikes
    ) const;

    // 기회를 초기화하고 새 라운드를 준비합니다.
    void ResetGame();

    // 모든 참가자가 기회를 소진했는지 확인합니다.
    void CheckDraw();

    // 결과를 공지하고 재시작을 예약합니다.
    void FinishRound(const FString& Message);

    // 클라이언트로 복제하지 않는 비밀 정답입니다.
    FString SecretNumber;

    // 약한 참조는 플레이어 객체가 사라졌는지 확인할 수 있습니다.
    TArray<TWeakObjectPtr<AMyPlayerState>> Players;

    // 결과를 5초 보여준 뒤 재시작할 타이머입니다.
    FTimerHandle ResetTimer;

    // Player 1, Player 2처럼 이름을 붙일 때 사용합니다.
    int32 NextPlayerNumber = 1;
};