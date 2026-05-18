// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameOverWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Shuai7Character.h"

void UGameOverWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnMainMenuClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnQuitClicked);
	}
}

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			PC->SetInputMode(FInputModeUIOnly());
			PC->bShowMouseCursor = true;
			UGameplayStatics::SetGlobalTimeDilation(World, 0.0f);
		}
	}

	UpdateStats();
}

void UGameOverWidget::UpdateStats()
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			AShuai7Character* Player = Cast<AShuai7Character>(PC->GetPawn());
			if (Player)
			{
				if (LevelText)
				{
					LevelText->SetText(FText::FromString(FString::Printf(TEXT("等级: %d"), Player->GetPlayerLevel())));
				}
			}
		}
	}
}

void UGameOverWidget::OnRestartClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()));
}

void UGameOverWidget::OnMainMenuClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}

void UGameOverWidget::OnQuitClicked()
{
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
