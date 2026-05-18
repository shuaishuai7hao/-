// Copyright Epic Games, Inc. All Rights Reserved.

#include "Shuai7GameHUD.h"
#include "MainMenuWidget.h"
#include "PauseMenuWidget.h"
#include "GameOverWidget.h"
#include "GameHUDWidget.h"
#include "MissionCompleteWidget.h"
#include "MissionFailedWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"

AShuai7GameHUD::AShuai7GameHUD()
{
}

void AShuai7GameHUD::BeginPlay()
{
	Super::BeginPlay();

	// 游戏开始时显示主菜单
	ShowMainMenu();
}

void AShuai7GameHUD::ShowMainMenu()
{
	if (MainMenuWidgetClass && !MainMenuWidget)
	{
		MainMenuWidget = CreateWidget<UMainMenuWidget>(GetWorld(), MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport();
		}
	}
}

void AShuai7GameHUD::HideMainMenu()
{
	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromViewport();
		MainMenuWidget = nullptr;
	}
}

void AShuai7GameHUD::ShowPauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromViewport();
		PauseMenuWidget = nullptr;
	}
	else if (PauseMenuWidgetClass)
	{
		PauseMenuWidget = CreateWidget<UPauseMenuWidget>(GetWorld(), PauseMenuWidgetClass);
		if (PauseMenuWidget)
		{
			PauseMenuWidget->AddToViewport();
		}
	}
}

void AShuai7GameHUD::HidePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromViewport();
		PauseMenuWidget = nullptr;
	}
}

void AShuai7GameHUD::ShowGameOver()
{
	if (GameOverWidgetClass && !GameOverWidget)
	{
		GameOverWidget = CreateWidget<UGameOverWidget>(GetWorld(), GameOverWidgetClass);
		if (GameOverWidget)
		{
			GameOverWidget->AddToViewport();
		}
	}
}

void AShuai7GameHUD::HideGameOver()
{
	if (GameOverWidget)
	{
		GameOverWidget->RemoveFromViewport();
		GameOverWidget = nullptr;
	}
}

void AShuai7GameHUD::ShowGameHUD()
{
	if (GameHUDWidgetClass && !GameHUDWidget)
	{
		GameHUDWidget = CreateWidget<UGameHUDWidget>(GetWorld(), GameHUDWidgetClass);
		if (GameHUDWidget)
		{
			GameHUDWidget->AddToViewport();
		}
	}
}

void AShuai7GameHUD::HideGameHUD()
{
	if (GameHUDWidget)
	{
		GameHUDWidget->RemoveFromViewport();
		GameHUDWidget = nullptr;
	}
}

void AShuai7GameHUD::ShowMissionComplete()
{
	if (!MissionCompleteWidgetClass)
	{
		return;
	}
	
	if (MissionCompleteWidget)
	{
		return;
	}
	
	if (MissionCompleteWidgetClass && !MissionCompleteWidget)
	{
		MissionCompleteWidget = CreateWidget<UMissionCompleteWidget>(GetWorld(), MissionCompleteWidgetClass);
		
		if (!MissionCompleteWidget)
		{
			return;
		}
		
		if (MissionCompleteWidget)
		{
			MissionCompleteWidget->AddToViewport();
		}
	}
}

void AShuai7GameHUD::ShowMissionFailed()
{
	if (MissionFailedWidgetClass && !MissionFailedWidget)
	{
		MissionFailedWidget = CreateWidget<UMissionFailedWidget>(GetWorld(), MissionFailedWidgetClass);
		if (MissionFailedWidget)
		{
			MissionFailedWidget->AddToViewport();
		}
	}
}

void AShuai7GameHUD::HideMissionFailed()
{
	if (MissionFailedWidget)
	{
		MissionFailedWidget->RemoveFromViewport();
		MissionFailedWidget = nullptr;
	}
}

void AShuai7GameHUD::UpdateHUD()
{
	if (GameHUDWidget)
	{
		GameHUDWidget->UpdateHUD();
	}
}