// Copyright Epic Games, Inc. All Rights Reserved.

#include "PauseMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "AudioManager.h"

void UPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	}

	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestartClicked);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
	}

	if (VolumeUpButton)
	{
		VolumeUpButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnVolumeUpClicked);
	}

	if (VolumeDownButton)
	{
		VolumeDownButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnVolumeDownClicked);
	}

	UpdateVolumeDisplay();
}

void UPauseMenuWidget::NativeConstruct()
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
}

void UPauseMenuWidget::NativeDestruct()
{
	Super::NativeDestruct();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
			UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
		}
	}
}

void UPauseMenuWidget::OnResumeClicked()
{
	RemoveFromParent();
}

void UPauseMenuWidget::OnRestartClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()));
}

void UPauseMenuWidget::OnMainMenuClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}

void UPauseMenuWidget::OnQuitClicked()
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

void UPauseMenuWidget::OnVolumeUpClicked()
{
	if (UAudioManager* AudioManager = UAudioManager::GetInstance())
	{
		AudioManager->IncreaseVolume();
		UpdateVolumeDisplay();
	}
}

void UPauseMenuWidget::OnVolumeDownClicked()
{
	if (UAudioManager* AudioManager = UAudioManager::GetInstance())
	{
		AudioManager->DecreaseVolume();
		UpdateVolumeDisplay();
	}
}

void UPauseMenuWidget::UpdateVolumeDisplay()
{
	if (VolumeText)
	{
		if (UAudioManager* AudioManager = UAudioManager::GetInstance())
		{
			float Volume = AudioManager->GetMusicVolume();
			VolumeText->SetText(FText::FromString(FString::Printf(TEXT("音量: %d%%"), (int32)(Volume * 100))));
		}
	}
}
