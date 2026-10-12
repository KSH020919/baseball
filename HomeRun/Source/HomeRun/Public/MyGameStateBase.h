#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameStateBase.generated.h"

// 게임이 어떤 단계에 있는지 나타냅니다.
UENUM()
enum class EBaseballPhase : uint8
{
    Waiting,  // 인원 대기
    Playing,  // 정답 입력 가능
    Finished  // 결과 표시 중
};

// 공용 정보를 보관합니다. 비밀 정답은 여기에 저장하지 않습니다.
UCLASS()
class HOMERUN_API AMyGameStateBase : public AGameStateBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    // GameMode가 서버에서 변경하고, UI가 클라이언트에서 읽습니다.
    UPROPERTY(Replicated)
    EBaseballPhase Phase = EBaseballPhase::Waiting;

    // 이전 라운드에서 늦게 도착한 입력을 구분할 때 사용합니다.
    UPROPERTY(Replicated)
    int32 RoundNumber = 0;

    // 화면에 표시할 게임 진행 안내 또는 결과입니다.
    UPROPERTY(Replicated)
    FString Announcement =
        TEXT("Waiting for at least 2 players.");

    // 서버가 호출하면 서버와 연결된 클라이언트에서 실행됩니다.
    UFUNCTION(NetMulticast, Reliable)
    void MulticastMessage(const FString& Message);
};