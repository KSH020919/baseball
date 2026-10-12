#pragma once // 이 헤더를 중복으로 포함하지 않도록 합니다.

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h" // generated.h는 마지막 include입니다.

// 플레이어 한 명의 시도 횟수를 저장하는 클래스입니다.
UCLASS()
class HOMERUN_API AMyPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    // 서버에서 클라이언트로 복제할 변수를 등록합니다.
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    // 현재 사용한 기회를 읽습니다.
    int32 GetCurrentAttemptCount() const
    {
        return Attempts;
    }

    // 최대 기회는 3번입니다.
    int32 GetMaxAttemptCount() const
    {
        return 3;
    }

    // 이번 라운드에 참가 중인지 확인합니다.
    bool IsParticipating() const
    {
        return bParticipating;
    }

    // 참가 중이고 기회가 남아 있는지 확인합니다.
    bool HasAttemptsRemaining() const;

    // 아래 두 함수는 서버에서만 실제 값을 변경합니다.
    void ResetForRound(bool bJoinRound);
    bool ConsumeAttempt();

private:
    // 서버에서 변경하면 클라이언트에도 전달됩니다.
    UPROPERTY(Replicated)
    int32 Attempts = 0;

    // 도중에 접속한 사람은 다음 라운드부터 참가합니다.
    UPROPERTY(Replicated)
    bool bParticipating = false;
};