// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioManager.h"
#include "Components/AudioComponent.h"

UAudioManager* UAudioManager::Instance = nullptr;

UAudioManager* UAudioManager::GetInstance()
{
	if (!Instance)
	{
		Instance = NewObject<UAudioManager>();
		Instance->AddToRoot();
	}
	return Instance;
}

void UAudioManager::PlayBackgroundMusic(USoundBase* Sound, UWorld* World)
{
	if (BackgroundMusicComponent)
	{
		BackgroundMusicComponent->Stop();
		BackgroundMusicComponent = nullptr;
	}

	if (Sound && World)
	{
		BackgroundMusicComponent = UGameplayStatics::SpawnSound2D(World, Sound, MusicVolume);
		if (BackgroundMusicComponent)
		{
			BackgroundMusicComponent->Play();
		}
	}
}

void UAudioManager::StopBackgroundMusic()
{
	if (BackgroundMusicComponent)
	{
		BackgroundMusicComponent->Stop();
		BackgroundMusicComponent = nullptr;
	}
}

void UAudioManager::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (BackgroundMusicComponent)
	{
		BackgroundMusicComponent->SetVolumeMultiplier(MusicVolume);
	}
}

void UAudioManager::IncreaseVolume()
{
	MusicVolume = FMath::Min(MusicVolume + 0.1f, 1.0f);
	if (BackgroundMusicComponent)
	{
		// 确保音乐在播放状态
		if (!BackgroundMusicComponent->IsPlaying())
		{
			BackgroundMusicComponent->Play();
		}
		BackgroundMusicComponent->SetVolumeMultiplier(MusicVolume);
	}
}

void UAudioManager::DecreaseVolume()
{
	MusicVolume = FMath::Max(MusicVolume - 0.1f, 0.0f);
	if (BackgroundMusicComponent)
	{
		// 即使音量为0，也保持播放状态（不停止）
		if (!BackgroundMusicComponent->IsPlaying())
		{
			BackgroundMusicComponent->Play();
		}
		BackgroundMusicComponent->SetVolumeMultiplier(MusicVolume);
	}
}
