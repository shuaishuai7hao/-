// Copyright Epic Games, Inc. All Rights Reserved.

#include "MissionCheckNPC.h"
#include "Shuai7GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

AMissionCheckNPC::AMissionCheckNPC()
{
}

void AMissionCheckNPC::BeginPlay()
{
	Super::BeginPlay();
}

void AMissionCheckNPC::OnPlayerInteract()
{
	// 播放交互音频（检查距离）
	if (InteractionSound && IsWithinAudioRange())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), InteractionSound, GetActorLocation());
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

	// 检查任务状态并显示结果
	CheckAndShowMissionStatus();
}

AShuai7GameMode* AMissionCheckNPC::GetGameMode() const
{
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld());
	if (GameMode)
	{
		return Cast<AShuai7GameMode>(GameMode);
	}
	return nullptr;
}

void AMissionCheckNPC::CheckAndShowMissionStatus()
{
	AShuai7GameMode* MissionGameMode = Cast<AShuai7GameMode>(GetWorld()->GetAuthGameMode());

	if (!MissionGameMode)
	{
		return;
	}
	
	bool bMission1Done = MissionGameMode->IsMission1Complete();
	bool bMission2Done = MissionGameMode->IsMission2Complete();

	if (bMission1Done && bMission2Done)
	{
		// 两个任务都完成，播放任务完成音效
		if (MissionCompleteSound && IsWithinAudioRange())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), MissionCompleteSound, GetActorLocation());
		}
		// 显示成功UI
		MissionGameMode->ShowMissionComplete();
	}
	else
	{
		// 任务未完成，显示提示UI
		MissionGameMode->ShowMissionFailed();
	}
}