// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class AShuai7Character;
class AShuai7GameMode;

UCLASS()
class SHUAI7_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHUD();

private:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ExpBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExpText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Skill1CooldownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Skill2CooldownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Skill3CooldownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Skill1StatusText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Skill2StatusText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Skill3StatusText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillAttackCooldownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillMissionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DialogueMissionText;

	TWeakObjectPtr<AShuai7Character> PlayerCharacter;
	TWeakObjectPtr<AShuai7GameMode> GameMode;

	void UpdateHealth();
	void UpdateExp();
	void UpdateSkillCooldowns();
	void UpdateMissionProgress();
};