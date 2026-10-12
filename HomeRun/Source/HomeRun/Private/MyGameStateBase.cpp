#include "MyGameStateBase.h"
#include "MyPlayerController.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

void AMyGameStateBase::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 진행 단계, 라운드 번호, 공지 내용을 공유합니다.
    DOREPLIFETIME(AMyGameStateBase, Phase);
    DOREPLIFETIME(AMyGameStateBase, RoundNumber);
    DOREPLIFETIME(AMyGameStateBase, Announcement);
}

void AMyGameStateBase::MulticastMessage_Implementation(
    const FString& Message
)
{
    // 각 컴퓨터에서 자신이 조작하는 컨트롤러를 찾습니다.
    for (
        auto It = GetWorld()->GetPlayerControllerIterator();
        It;
        ++It
        )
    {
        AMyPlayerController* PC =
            Cast<AMyPlayerController>(It->Get());

        // 본인의 화면에만 메시지를 표시합니다.
        if (PC && PC->IsLocalController())
        {
            PC->ReceiveMessage(Message);
        }
    }
}