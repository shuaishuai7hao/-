// Copyright Epic Games, Inc. All Rights Reserved.

#include "DialogueNPC.h"
#include "Shuai7Character.h"
#include "Shuai7GameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/AudioComponent.h"

ADialogueNPC::ADialogueNPC()
{
	PrimaryActorTick.bCanEverTick = true;

	// 禁用移动
	GetCharacterMovement()->SetMovementMode(MOVE_None);

	// 设置碰撞
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("NPC"));
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
}

void ADialogueNPC::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = Cast<AShuai7Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// 禁用重力，固定NPC在当前位置
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GravityScale = 0.f;
		GetCharacterMovement()->SetMovementMode(MOVE_None);
	}
}

void ADialogueNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePlayerProximity();
}

void ADialogueNPC::UpdatePlayerProximity()
{
	if (PlayerRef)
	{
		float Distance = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());
		bIsPlayerNearby = Distance <= InteractionRadius;
	}
}

void ADialogueNPC::ShowDialogue()
{
	if (DialogueWidgetClass && !DialogueWidget)
	{
		DialogueWidget = CreateWidget<UUserWidget>(GetWorld(), DialogueWidgetClass);
		if (DialogueWidget)
		{
			DialogueWidget->AddToViewport();

			// 设置UI中的文本
			UTextBlock* DialogueTextWidget = Cast<UTextBlock>(DialogueWidget->GetWidgetFromName(TEXT("DialogueText")));
			if (DialogueTextWidget)
			{
				DialogueTextWidget->SetText(DialogueText);
			}

			UTextBlock* NameTextWidget = Cast<UTextBlock>(DialogueWidget->GetWidgetFromName(TEXT("NPCName")));
			if (NameTextWidget)
			{
				NameTextWidget->SetText(NPCName);
			}
		}
	}
}

void ADialogueNPC::HideDialogue()
{
	if (DialogueWidget)
	{
		DialogueWidget->RemoveFromViewport();
		DialogueWidget = nullptr;
	}
}

void ADialogueNPC::OnPlayerInteract()
{
	if (DialogueWidget)
	{
		// 隐藏UI时停止语音
		HideDialogue();

		// 立即停止音频
		if (ActiveDialogueAudio)
		{
			ActiveDialogueAudio->Stop();
			ActiveDialogueAudio = nullptr;
		}
	}
	else
	{
		// 只在第一次打开时播放音频
		if (!bHasPlayedDialogueSound)
		{
			// 播放交互音频（检查距离）
			if (InteractionSound && IsWithinAudioRange())
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), InteractionSound, GetActorLocation());
			}

			// 播放对话语音（检查距离）
			if (DialogueSound && IsWithinAudioRange())
			{
				ActiveDialogueAudio = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), DialogueSound, GetActorLocation());
			}

			bHasPlayedDialogueSound = true; // 标记已播放

			// 通知任务系统对话完成（只在第一次对话时通知）
			if (!bHasCompletedDialogue)
			{
				bHasCompletedDialogue = true;

				AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld());
				if (GameMode)
				{
					AShuai7GameMode* MissionGameMode = Cast<AShuai7GameMode>(GameMode);
					if (MissionGameMode)
					{
						MissionGameMode->OnDialogueCompleted();
					}
				}
			}
		}

		// 显示UI
		ShowDialogue();
	}

	// 播放对话蒙太奇动画（每次交互都播放）
	if (DialogueMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			PlayAnimMontage(DialogueMontage);
		}
	}
}

bool ADialogueNPC::IsWithinAudioRange() const
{
	if (PlayerRef)
	{
		float Distance = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());
		return Distance <= MaxAudioDistance;
	}
	return true;
}