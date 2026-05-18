// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Shuai7GameHUD.generated.h"

class UMainMenuWidget;
class UPauseMenuWidget;
class UGameOverWidget;
class UGameHUDWidget;
class UMissionCompleteWidget;
class UMissionFailedWidget;

UCLASS()
class SHUAI7_API AShuai7GameHUD : public AHUD
{
	GENERATED_BODY()

public:
	AShuai7GameHUD();

	void ShowMainMenu();
	void HideMainMenu();

	void ShowPauseMenu();
	void HidePauseMenu();

	void ShowGameOver();
	void HideGameOver();

	void ShowGameHUD();
	void HideGameHUD();

	void ShowMissionComplete();
	void ShowMissionFailed();
	void HideMissionFailed();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHUD();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UGameOverWidget> GameOverWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UGameHUDWidget> GameHUDWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UMissionCompleteWidget> MissionCompleteWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UMissionFailedWidget> MissionFailedWidgetClass;

	UPROPERTY()
	UMainMenuWidget* MainMenuWidget;

	UPROPERTY()
	UPauseMenuWidget* PauseMenuWidget;

	UPROPERTY()
	UGameOverWidget* GameOverWidget;

	UPROPERTY()
	UGameHUDWidget* GameHUDWidget;

	UPROPERTY()
	UMissionCompleteWidget* MissionCompleteWidget;

	UPROPERTY()
	UMissionFailedWidget* MissionFailedWidget;
};