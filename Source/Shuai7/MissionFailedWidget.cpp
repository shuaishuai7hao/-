// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionFailedWidget.h"
#include "Shuai7GameHUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UMissionFailedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定按钮点击事件
	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddDynamic(this, &UMissionFailedWidget::OnContinueClicked);
	}
}

void UMissionFailedWidget::OnContinueClicked()
{
	// 恢复游戏时间
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	// 隐藏鼠标
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	// 获取HUD并隐藏自己
	if (PC)
	{
		AShuai7GameHUD* HUD = Cast<AShuai7GameHUD>(PC->GetHUD());
		if (HUD)
		{
			HUD->HideMissionFailed();
		}
	}
}