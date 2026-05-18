// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class SHUAI7_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UButton* ResumeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* RestartButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MainMenuButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
	UButton* VolumeUpButton;

	UPROPERTY(meta = (BindWidget))
	UButton* VolumeDownButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PauseText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* VolumeText;

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnMainMenuClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnVolumeUpClicked();

	UFUNCTION()
	void OnVolumeDownClicked();

	void UpdateVolumeDisplay();
};
