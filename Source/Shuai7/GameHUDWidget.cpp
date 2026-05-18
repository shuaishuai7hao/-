// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Shuai7Character.h"
#include "Shuai7GameMode.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UGameHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			PlayerCharacter = Cast<AShuai7Character>(PC->GetPawn());
		}

		AGameModeBase* GM = UGameplayStatics::GetGameMode(World);
		if (GM)
		{
			GameMode = Cast<AShuai7GameMode>(GM);
		}
	}
}

void UGameHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	static int32 UpdateCounter = 0;
	UpdateCounter++;

	if (UpdateCounter >= 5)
	{
		UpdateCounter = 0;
		UpdateHUD();
	}
}

void UGameHUDWidget::UpdateHUD()
{
	UpdateHealth();
	UpdateExp();
	UpdateSkillCooldowns();
	UpdateMissionProgress();
}

void UGameHUDWidget::UpdateHealth()
{
	if (!PlayerCharacter.IsValid())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			if (PC)
			{
				PlayerCharacter = Cast<AShuai7Character>(PC->GetPawn());
			}
		}
	}

	if (PlayerCharacter.IsValid())
	{
		float HealthPercent = PlayerCharacter->GetCurrentHealth() / PlayerCharacter->GetMaxHealth();

		if (HealthBar)
		{
			HealthBar->SetPercent(HealthPercent);
		}

		if (HealthText)
		{
			HealthText->SetText(FText::FromString(
				FString::Printf(TEXT("%.0f / %.0f"),
					PlayerCharacter->GetCurrentHealth(),
					PlayerCharacter->GetMaxHealth())));
		}
	}
}

void UGameHUDWidget::UpdateExp()
{
	if (!PlayerCharacter.IsValid())
	{
		return;
	}

	int32 CurrentExp = PlayerCharacter->GetExp();
	int32 MaxExp = PlayerCharacter->GetExpToNextLevel();
	float ExpPercent = MaxExp > 0 ? (float)CurrentExp / (float)MaxExp : 0.f;

	if (ExpBar)
	{
		ExpBar->SetPercent(ExpPercent);
	}

	if (ExpText)
	{
		ExpText->SetText(FText::FromString(
			FString::Printf(TEXT("经验: %d / %d"),
				CurrentExp,
				MaxExp)));
	}

	if (LevelText)
	{
		LevelText->SetText(FText::FromString(
			FString::Printf(TEXT("等级 %d"), PlayerCharacter->GetPlayerLevel())));
	}
}

void UGameHUDWidget::UpdateSkillCooldowns()
{
	if (!PlayerCharacter.IsValid())
	{
		return;
	}

	// Skill 1 (Q)
	if (Skill1StatusText)
	{
		Skill1StatusText->SetText(FText::FromString(
			PlayerCharacter->IsSkill1Unlocked() ? TEXT("[Q]") : TEXT("[ ]")));
	}

	if (Skill1CooldownText)
	{
		float Cooldown = PlayerCharacter->GetSkill1CooldownRemaining();
		if (Cooldown > 0.f)
		{
			Skill1CooldownText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Cooldown)));
			Skill1CooldownText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Skill1CooldownText->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// Skill 2 (Shift)
	if (Skill2StatusText)
	{
		Skill2StatusText->SetText(FText::FromString(
			PlayerCharacter->IsSkill2Unlocked() ? TEXT("[Shift]") : TEXT("[     ]")));
	}

	if (Skill2CooldownText)
	{
		float Cooldown = PlayerCharacter->GetSkill2CooldownRemaining();
		if (Cooldown > 0.f)
		{
			Skill2CooldownText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Cooldown)));
			Skill2CooldownText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Skill2CooldownText->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// Skill 3 (R)
	if (Skill3StatusText)
	{
		Skill3StatusText->SetText(FText::FromString(
			PlayerCharacter->IsSkill3Unlocked() ? TEXT("[R]") : TEXT("[ ]")));
	}

	if (Skill3CooldownText)
	{
		float Cooldown = PlayerCharacter->GetSkill3CooldownRemaining();
		if (Cooldown > 0.f)
		{
			Skill3CooldownText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Cooldown)));
			Skill3CooldownText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Skill3CooldownText->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// Skill Attack (E)
	if (SkillAttackCooldownText)
	{
		float Cooldown = PlayerCharacter->GetSkillAttackCooldownRemaining();
		if (Cooldown > 0.f)
		{
			SkillAttackCooldownText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Cooldown)));
			SkillAttackCooldownText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SkillAttackCooldownText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UGameHUDWidget::UpdateMissionProgress()
{
	if (!GameMode.IsValid())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			AGameModeBase* GM = UGameplayStatics::GetGameMode(World);
			if (GM)
			{
				GameMode = Cast<AShuai7GameMode>(GM);
			}
		}
	}

	if (GameMode.IsValid())
	{
		int32 KillCount = GameMode->GetKillCount();
		int32 RequiredKills = GameMode->RequiredKillCount;

		int32 DialogueCount = GameMode->GetDialogueCount();
		int32 RequiredDialogues = GameMode->RequiredDialogueCount;

		if (KillMissionText)
		{
			FString KillText = FString::Printf(TEXT("击杀: %d/%d"), KillCount, RequiredKills);
			KillMissionText->SetText(FText::FromString(KillText));
		}

		if (DialogueMissionText)
		{
			FString DialogueText = FString::Printf(TEXT("对话: %d/%d"), DialogueCount, RequiredDialogues);
			DialogueMissionText->SetText(FText::FromString(DialogueText));
		}
	}
}