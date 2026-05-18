// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Shuai7GameMode.generated.h"

class AShuai7GameHUD;
class USoundBase;

UCLASS()
class SHUAI7_API AShuai7GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AShuai7GameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* BackgroundMusic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 RequiredKillCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 RequiredDialogueCount = 3;

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void HidePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ShowGameOver();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnPlayerDeath();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ShowMissionComplete();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ShowMissionFailed();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void RestartGame();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void QuitGame();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void OnEnemyKilled();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void OnDialogueCompleted();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void CheckMissionComplete();

	UFUNCTION(BlueprintPure, Category = "Mission")
	int32 GetKillCount() const { return KillCount; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	int32 GetDialogueCount() const { return DialogueCount; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsMission1Complete() const { return bMission1Complete; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsMission2Complete() const { return bMission2Complete; }

protected:
	virtual void BeginPlay() override;

	void ForceLoadAllStreamingLevels();

private:
	AShuai7GameHUD* GameHUD;

	int32 KillCount;
	int32 DialogueCount;
	bool bMission1Complete;
	bool bMission2Complete;
	bool bAllMissionsComplete;
};