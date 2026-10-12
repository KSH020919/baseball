#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UMyUserWidget;

// 클라이언트의 입력을 서버로 보내는 통신 창구입니다.
UCLASS()
class HOMERUN_API AMyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    virtual void EndPlay(
        const EEndPlayReason::Type Reason
    ) override;

    // UI가 정답을 제출할 때 호출합니다.
    void SendGuess(const FString& Input);

    // UI가 일반 채팅을 보낼 때 호출합니다.
    void SendChat(const FString& Input);

    // 받은 메시지를 로컬 UI에 추가합니다.
    void ReceiveMessage(const FString& Message);

    // 이 컨트롤러를 소유한 클라이언트에게만 안내를 보냅니다.
    UFUNCTION(Client, Reliable)
    void ClientNotice(const FString& Message);

private:
    // 클라이언트가 호출해도 실제 처리는 서버가 합니다.
    UFUNCTION(Server, Reliable)
    void ServerSubmitGuess(
        const FString& Input,
        int32 RoundNumber
    );

    UFUNCTION(Server, Reliable)
    void ServerSubmitChat(const FString& Input);

    // 복제된 최신 상태를 화면에 표시합니다.
    void RefreshUI();

    // 화면 객체의 참조를 유지합니다.
    UPROPERTY(Transient)
    TObjectPtr<UMyUserWidget> GameWidget;

    // 화면 생성 전에 도착한 메시지를 잠시 저장합니다.
    TArray<FString> PendingMessages;

    // 주기적으로 화면을 갱신할 타이머입니다.
    FTimerHandle UITimer;

    // 과도하게 빠른 반복 요청을 제한할 때 사용합니다.
    double LastGuessTime = -100.0;
    double LastChatTime = -100.0;
};