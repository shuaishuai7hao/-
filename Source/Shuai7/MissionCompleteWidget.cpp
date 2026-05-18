// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionCompleteWidget.h"
#include "Shuai7GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UMissionCompleteWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定按钮点击事件
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UMissionCompleteWidget::OnRestartClicked);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UMissionCompleteWidget::OnMainMenuClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UMissionCompleteWidget::OnQuitClicked);
	}
}

void UMissionCompleteWidget::OnRestartClicked()
{
	// 获取GameMode并调用重新开始
	AShuai7GameMode* GameMode = Cast<AShuai7GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		GameMode->RestartGame();
	}
}

void UMissionCompleteWidget::OnMainMenuClicked()
{
	// 获取GameMode并调用返回主菜单
	AShuai7GameMode* GameMode = Cast<AShuai7GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		GameMode->ReturnToMainMenu();
	}
}

void UMissionCompleteWidget::OnQuitClicked()
{
	// 获取GameMode并调用退出游戏
	AShuai7GameMode* GameMode = Cast<AShuai7GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		GameMode->QuitGame();
	}
}