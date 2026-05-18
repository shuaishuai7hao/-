// Copyright Epic Games, Inc. All Rights Reserved.

#include "Shuai7GameMode.h"
#include "Shuai7Character.h"
#include "Shuai7GameHUD.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "AudioManager.h"

AShuai7GameMode::AShuai7GameMode()
{
	// 设置默认 Pawn 类
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// 初始化任务变量
	KillCount = 0;
	DialogueCount = 0;
	bMission1Complete = false;
	bMission2Complete = false;
	bAllMissionsComplete = false;
}

void AShuai7GameMode::BeginPlay()
{
	Super::BeginPlay();

	// 强制加载所有流关卡（加载整个地图）
	ForceLoadAllStreamingLevels();

	// 确保游戏时间正常运行
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	// 确保输入模式正确
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	// 尝试获取自定义 HUD
	AHUD* CurrentHUD = GetWorld()->GetFirstPlayerController()->GetHUD();
	if (CurrentHUD)
	{
		GameHUD = Cast<AShuai7GameHUD>(CurrentHUD);
		if (GameHUD)
		{
			GameHUD->ShowGameHUD();
		}
	}

	// 播放背景音乐
	if (BackgroundMusic)
	{
		if (UAudioManager* AudioManager = UAudioManager::GetInstance())
		{
			AudioManager->PlayBackgroundMusic(BackgroundMusic, GetWorld());
		}
	}
}

void AShuai7GameMode::ShowPauseMenu()
{
	if (GameHUD)
	{
		GameHUD->ShowPauseMenu();
	}
}

void AShuai7GameMode::HidePauseMenu()
{
	if (GameHUD)
	{
		GameHUD->HidePauseMenu();
	}
}

void AShuai7GameMode::ShowGameOver()
{
	if (GameHUD)
	{
		GameHUD->HideGameHUD();
		GameHUD->ShowGameOver();
	}
}

void AShuai7GameMode::OnPlayerDeath()
{
	ShowGameOver();
}

void AShuai7GameMode::ShowMissionComplete()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		// 停止游戏时间
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);

		// 显示鼠标
		PC->bShowMouseCursor = true;

		// 设置输入模式为UI
		PC->SetInputMode(FInputModeUIOnly());

		AShuai7GameHUD* HUD = Cast<AShuai7GameHUD>(PC->GetHUD());
		if (HUD)
		{
			HUD->HideGameHUD();
			HUD->ShowMissionComplete();
		}
	}
}

void AShuai7GameMode::ShowMissionFailed()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		// 停止游戏时间
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);

		// 显示鼠标
		PC->bShowMouseCursor = true;

		// 设置输入模式为UI
		PC->SetInputMode(FInputModeUIOnly());

		AShuai7GameHUD* HUD = Cast<AShuai7GameHUD>(PC->GetHUD());
		if (HUD)
		{
			HUD->ShowMissionFailed();
		}
	}
}

void AShuai7GameMode::RestartGame()
{
	// 恢复游戏时间
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	// 重新加载当前关卡
	UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()));
}

void AShuai7GameMode::ReturnToMainMenu()
{
	// 恢复游戏时间
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	// 返回主菜单
	UGameplayStatics::OpenLevel(GetWorld(), FName("MainMenu"));
}

void AShuai7GameMode::QuitGame()
{
	// 获取平台特定的游戏实例
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			PC->ConsoleCommand("quit");
		}
	}
}

void AShuai7GameMode::OnEnemyKilled()
{
	KillCount++;
	CheckMissionComplete();
}

void AShuai7GameMode::OnDialogueCompleted()
{
	DialogueCount++;
	CheckMissionComplete();
}

void AShuai7GameMode::CheckMissionComplete()
{
	if (!bMission1Complete && KillCount >= RequiredKillCount)
	{
		bMission1Complete = true;
	}

	if (!bMission2Complete && DialogueCount >= RequiredDialogueCount)
	{
		bMission2Complete = true;
	}

	if (!bAllMissionsComplete && bMission1Complete && bMission2Complete)
	{
		bAllMissionsComplete = true;
	}
}

void AShuai7GameMode::ForceLoadAllStreamingLevels()
{
	if (!GetWorld()) return;

	// 获取所有流关卡
	TArray<ULevelStreaming*> StreamingLevels = GetWorld()->GetStreamingLevels();
	
	for (ULevelStreaming* Level : StreamingLevels)
	{
		if (Level)
		{
			// 强制加载关卡
			Level->SetShouldBeLoaded(true);
			Level->SetShouldBeVisible(true);
		}
	}

	// 等待所有关卡加载完成
	GetWorld()->FlushLevelStreaming(EFlushLevelStreamingType::Full);
}