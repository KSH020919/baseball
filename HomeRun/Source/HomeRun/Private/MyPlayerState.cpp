#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME에 필요합니다.

void AMyPlayerState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    // 부모 클래스의 복제 설정도 유지합니다.
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 시도 횟수와 참가 상태를 클라이언트에 복제합니다.
    DOREPLIFETIME(AMyPlayerState, Attempts);
    DOREPLIFETIME(AMyPlayerState, bParticipating);
}

bool AMyPlayerState::HasAttemptsRemaining() const
{
    // 참가 중이며 사용한 기회가 3번 미만이어야 합니다.
    return bParticipating && Attempts < GetMaxAttemptCount();
}

void AMyPlayerState::ResetForRound(bool bJoinRound)
{
    // 서버가 아니면 상태를 변경하지 않습니다.
    if (!HasAuthority()) return;

    Attempts = 0;
    bParticipating = bJoinRound;

    // 변경 내용을 빠르게 전송하도록 요청합니다.
    ForceNetUpdate();
}

bool AMyPlayerState::ConsumeAttempt()
{
    // 서버인지, 기회가 남았는지 다시 확인합니다.
    if (!HasAuthority() || !HasAttemptsRemaining()) return false;

    // 사용한 기회를 1회 증가시킵니다.
    ++Attempts;

    ForceNetUpdate();

    // 기회 차감에 성공했습니다.
    return true;
}