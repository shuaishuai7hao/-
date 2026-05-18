// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "AudioManager.generated.h"

UCLASS()
class SHUAI7_API UAudioManager : public UObject
{
	GENERATED_BODY()

private:
	UAudioManager() {}

	static UAudioManager* Instance;

	UPROPERTY()
	UAudioComponent* BackgroundMusicComponent;

	float MusicVolume = 0.5f;

public:
	static UAudioManager* GetInstance();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayBackgroundMusic(class USoundBase* Sound, UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopBackgroundMusic();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	float GetMusicVolume() const { return MusicVolume; }

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void IncreaseVolume();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void DecreaseVolume();
};
