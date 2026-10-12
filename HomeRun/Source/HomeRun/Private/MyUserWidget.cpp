#include "MyUserWidget.h"
#include "MyGameStateBase.h"
#include "MyPlayerController.h"
#include "MyPlayerState.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"

UTextBlock* UMyUserWidget::MakeText(
    const FString& Text,
    int32 Size
)
{
    // 글자를 표시할 위젯을 만듭니다.
    UTextBlock* Widget =
        WidgetTree->ConstructWidget<UTextBlock>();

    Widget->SetText(FText::FromString(Text));

    // 긴 문장은 다음 줄에 표시합니다.
    Widget->SetAutoWrapText(true);

    Widget->SetColorAndOpacity(
        FSlateColor(FLinearColor::White)
    );

    // 기본 폰트를 유지하면서 크기만 변경합니다.
    FSlateFontInfo Font = Widget->GetFont();
    Font.Size = Size;
    Widget->SetFont(Font);

    return Widget;
}

UEditableTextBox* UMyUserWidget::MakeInput(
    UVerticalBox* Parent,
    const FString& Hint,
    const FString& Label,
    UButton*& Button
)
{
    // 입력창과 버튼을 가로로 배치합니다.
    UHorizontalBox* Row =
        WidgetTree->ConstructWidget<UHorizontalBox>();

    Parent->AddChildToVerticalBox(Row)
        ->SetPadding(FMargin(0, 5));

    UEditableTextBox* Input =
        WidgetTree->ConstructWidget<UEditableTextBox>();

    // 어두운 배경과 흰 글자로 입력창의 가독성을 높입니다.
    FEditableTextBoxStyle& Style = Input->WidgetStyle;

    const FSlateColor Background(
        FLinearColor(0.012f, 0.018f, 0.028f, 1.0f)
    );

    const FSlateColor White(FLinearColor::White);

    // 기본, 마우스 올림, 선택, 읽기 전용 상태의 배경입니다.
    Style.BackgroundImageNormal.TintColor = Background;
    Style.BackgroundImageHovered.TintColor = Background;
    Style.BackgroundImageFocused.TintColor = Background;
    Style.BackgroundImageReadOnly.TintColor = Background;

    // 상태가 바뀌어도 글자가 흐려지지 않도록 지정합니다.
    Style.SetForegroundColor(White);
    Style.SetFocusedForegroundColor(White);
    Style.SetReadOnlyForegroundColor(White);

    Style.TextStyle.SetColorAndOpacity(White);
    Style.TextStyle.SetFontSize(18);
    Style.SetPadding(FMargin(12, 8));

    Input->SetForegroundColor(FLinearColor::White);

    // 빈 입력창에 표시할 안내 문구입니다.
    Input->SetHintText(FText::FromString(Hint));

    // Enter를 누른 뒤에도 계속 입력할 수 있게 합니다.
    Input->SetClearKeyboardFocusOnCommit(false);

    UHorizontalBoxSlot* InputSlot =
        Row->AddChildToHorizontalBox(Input);

    // 버튼을 제외한 가로 공간을 입력창이 사용합니다.
    InputSlot->SetSize(
        FSlateChildSize(ESlateSizeRule::Fill)
    );

    InputSlot->SetPadding(FMargin(0, 0, 10, 0));

    // 참조 매개변수로 버튼도 호출한 곳에 전달합니다.
    Button = WidgetTree->ConstructWidget<UButton>();

    Button->SetBackgroundColor(
        FLinearColor(0.08f, 0.35f, 0.22f)
    );

    Button->SetContent(MakeText(Label, 18));

    Row->AddChildToHorizontalBox(Button);

    return Input;
}

TSharedRef<SWidget> UMyUserWidget::RebuildWidget()
{
    // 위젯들을 보관할 트리가 없으면 생성합니다.
    if (!WidgetTree)
    {
        WidgetTree = NewObject<UWidgetTree>(
            this,
            TEXT("WidgetTree")
        );
    }

    // 이미 구성된 화면은 중복 생성하지 않습니다.
    if (!WidgetTree->RootWidget)
    {
        UBorder* Background =
            WidgetTree->ConstructWidget<UBorder>();

        Background->SetBrushColor(
            FLinearColor(0.025f, 0.035f, 0.06f, 1.0f)
        );

        Background->SetPadding(FMargin(24));

        WidgetTree->RootWidget = Background;

        // 위에서 아래로 위젯들을 배치합니다.
        UVerticalBox* Root =
            WidgetTree->ConstructWidget<UVerticalBox>();

        Background->SetContent(Root);

        // 게임 제목과 규칙입니다.
        Root->AddChildToVerticalBox(
            MakeText(TEXT("HOME RUN | Number Baseball"), 28)
        );

        Root->AddChildToVerticalBox(
            MakeText(
                TEXT("Digits 1-9 / No duplicates / 3 attempts per player"),
                16
            )
        );

        // 게임 진행과 결과를 표시하는 공지 영역입니다.
        StatusText = MakeText(
            TEXT("Connecting to server..."),
            22
        );

        StatusText->SetColorAndOpacity(
            FSlateColor(FLinearColor(1.0f, 0.8f, 0.3f))
        );

        Root->AddChildToVerticalBox(StatusText)
            ->SetPadding(FMargin(0, 12));

        // 각 플레이어의 시도 횟수 영역입니다.
        PlayersText = MakeText(TEXT(""), 18);

        Root->AddChildToVerticalBox(PlayersText);

        // 채팅과 판정 결과를 표시할 스크롤 영역입니다.
        HistoryScroll =
            WidgetTree->ConstructWidget<UScrollBox>();

        HistoryText = MakeText(TEXT(""), 16);

        HistoryScroll->AddChild(HistoryText);

        UVerticalBoxSlot* HistorySlot =
            Root->AddChildToVerticalBox(HistoryScroll);

        // 남는 세로 공간을 채팅 기록 영역이 사용합니다.
        HistorySlot->SetSize(
            FSlateChildSize(ESlateSizeRule::Fill)
        );

        HistorySlot->SetPadding(FMargin(0, 12));

        // 정답 입력창과 버튼을 만듭니다.
        UButton* SubmitButton = nullptr;

        GuessInput = MakeInput(
            Root,
            TEXT("Enter a guess (e.g. 386)"),
            TEXT("Submit guess"),
            SubmitButton
        );

        GuessButton = SubmitButton;

        // 버튼 클릭과 Enter 입력을 각각 함수에 연결합니다.
        GuessButton->OnClicked.AddDynamic(
            this,
            &UMyUserWidget::SubmitGuess
        );

        GuessInput->OnTextCommitted.AddDynamic(
            this,
            &UMyUserWidget::GuessCommitted
        );

        // 일반 채팅 입력창과 버튼을 만듭니다.
        UButton* ChatButton = nullptr;

        ChatInput = MakeInput(
            Root,
            TEXT("Chat (up to 200 characters)"),
            TEXT("Send chat"),
            ChatButton
        );

        ChatButton->OnClicked.AddDynamic(
            this,
            &UMyUserWidget::SubmitChat
        );

        ChatInput->OnTextCommitted.AddDynamic(
            this,
            &UMyUserWidget::ChatCommitted
        );
    }

    // 만든 위젯 트리를 실제 화면으로 구성합니다.
    return Super::RebuildWidget();
}

void UMyUserWidget::AppendMessage(
    const FString& Message
)
{
    // 새 메시지를 기록 끝에 추가합니다.
    Messages.Add(Message);

    // 최근 100개를 초과하면 가장 오래된 것을 지웁니다.
    if (Messages.Num() > 100)
    {
        Messages.RemoveAt(0);
    }

    if (HistoryText)
    {
        // 메시지들을 줄 바꿈으로 연결해서 표시합니다.
        HistoryText->SetText(
            FText::FromString(
                FString::Join(Messages, TEXT("\n"))
            )
        );
    }

    if (HistoryScroll)
    {
        // 최신 메시지가 보이도록 맨 아래로 이동합니다.
        HistoryScroll->ScrollToEnd();
    }
}

void UMyUserWidget::RefreshState()
{
    // 화면 구성 전에는 갱신하지 않습니다.
    if (!StatusText || !GuessButton) return;

    const AMyGameStateBase* GS =
        GetWorld()->GetGameState<AMyGameStateBase>();

    const AMyPlayerController* PC =
        Cast<AMyPlayerController>(GetOwningPlayer());

    const AMyPlayerState* Me = PC
        ? PC->GetPlayerState<AMyPlayerState>()
        : nullptr;

    // 게임 진행 중이며 자신의 기회가 남아 있어야 제출할 수 있습니다.
    const bool bCanGuess =
        GS &&
        Me &&
        GS->Phase == EBaseballPhase::Playing &&
        Me->HasAttemptsRemaining();

    GuessInput->SetIsEnabled(bCanGuess);
    GuessButton->SetIsEnabled(bCanGuess);

    // 서버 정보가 아직 도착하지 않았다면 다음 갱신을 기다립니다.
    if (!GS) return;

    StatusText->SetText(
        FText::FromString(GS->Announcement)
    );

    // 모든 플레이어의 이름과 시도 횟수를 모읍니다.
    TArray<FString> PlayerLines;

    for (APlayerState* Base : GS->PlayerArray)
    {
        const AMyPlayerState* PS =
            Cast<AMyPlayerState>(Base);

        if (!PS) continue;

        PlayerLines.Add(
            FString::Printf(
                TEXT("%s%s [%d / %d]%s"),
                PS == Me ? TEXT("You: ") : TEXT(""),
                *PS->GetPlayerName(),
                PS->GetCurrentAttemptCount(),
                PS->GetMaxAttemptCount(),
                PS->IsParticipating()
                ? TEXT("")
                : TEXT(" (waiting)")
            )
        );
    }

    // 문자열 순서로 정렬해서 목록 순서를 안정적으로 유지합니다.
    PlayerLines.Sort();

    PlayersText->SetText(
        FText::FromString(
            FString::Join(PlayerLines, TEXT("\n"))
        )
    );
}

void UMyUserWidget::SubmitGuess()
{
    if (!GuessInput || !GuessInput->GetIsEnabled())
    {
        return;
    }

    AMyPlayerController* PC =
        Cast<AMyPlayerController>(GetOwningPlayer());

    if (PC)
    {
        // 위젯에서는 판정하지 않고 입력만 전달합니다.
        PC->SendGuess(
            GuessInput->GetText().ToString()
        );
    }

    // 입력창을 비우고 다음 입력을 받을 준비를 합니다.
    GuessInput->SetText(FText::GetEmpty());
    GuessInput->SetKeyboardFocus();
}

void UMyUserWidget::SubmitChat()
{
    if (!ChatInput) return;

    AMyPlayerController* PC =
        Cast<AMyPlayerController>(GetOwningPlayer());

    if (PC)
    {
        // 일반 채팅은 정답 제출과 별도 경로로 보냅니다.
        PC->SendChat(
            ChatInput->GetText().ToString()
        );
    }

    ChatInput->SetText(FText::GetEmpty());
    ChatInput->SetKeyboardFocus();
}

void UMyUserWidget::GuessCommitted(
    const FText& Text,
    ETextCommit::Type Method
)
{
    // 포커스 이동이 아니라 Enter를 누른 경우에만 전송합니다.
    if (Method == ETextCommit::OnEnter)
    {
        SubmitGuess();
    }
}

void UMyUserWidget::ChatCommitted(
    const FText& Text,
    ETextCommit::Type Method
)
{
    if (Method == ETextCommit::OnEnter)
    {
        SubmitChat();
    }
}