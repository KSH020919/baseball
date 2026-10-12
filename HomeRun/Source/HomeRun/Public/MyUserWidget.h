#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "MyUserWidget.generated.h"

class UButton;
class UEditableTextBox;
class UScrollBox;
class UTextBlock;
class UVerticalBox;

// 입력창, 채팅 기록, 시도 횟수, 결과 공지를 표시합니다.
UCLASS()
class HOMERUN_API UMyUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 채팅과 판정 결과를 기록합니다.
    void AppendMessage(const FString& Message);

    // 복제된 게임 상태를 표시합니다.
    void RefreshState();

protected:
    // 위젯 블루프린트 없이 C++로 UI를 구성합니다.
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    // 반복되는 텍스트 위젯 생성 작업을 함수로 분리했습니다.
    UTextBlock* MakeText(
        const FString& Text,
        int32 Size
    );

    // 입력창과 버튼을 한 줄로 만듭니다.
    UEditableTextBox* MakeInput(
        UVerticalBox* Parent,
        const FString& Hint,
        const FString& Label,
        UButton*& Button
    );

    UFUNCTION()
    void SubmitGuess(); // 정답 제출 버튼

    UFUNCTION()
    void SubmitChat(); // 채팅 전송 버튼

    UFUNCTION()
    void GuessCommitted(
        const FText& Text,
        ETextCommit::Type Method
    );

    UFUNCTION()
    void ChatCommitted(
        const FText& Text,
        ETextCommit::Type Method
    );

    // 진행 상태와 승리/무승부 공지입니다.
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> StatusText;

    // 모든 플레이어의 시도 횟수입니다.
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PlayersText;

    // 채팅과 판정 기록을 하나의 텍스트 위젯에 표시합니다.
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> HistoryText;

    // 긴 기록을 스크롤할 수 있는 영역입니다.
    UPROPERTY(Transient)
    TObjectPtr<UScrollBox> HistoryScroll;

    UPROPERTY(Transient)
    TObjectPtr<UEditableTextBox> GuessInput;

    UPROPERTY(Transient)
    TObjectPtr<UEditableTextBox> ChatInput;

    // 기회를 소진하면 비활성화할 버튼입니다.
    UPROPERTY(Transient)
    TObjectPtr<UButton> GuessButton;

    // 최근 100개 메시지만 보관합니다.
    TArray<FString> Messages;
};