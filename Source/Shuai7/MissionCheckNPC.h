// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogueNPC.h"
#include "MissionCheckNPC.generated.h"

class AShuai7GameMode;
class USoundBase;

UCLASS()
class SHUAI7_API AMissionCheckNPC : public ADialogueNPC
{
	GENERATED_BODY()

public:
	AMissionCheckNPC();

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnPlayerInteract() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* MissionCompleteSound;

private:
	AShuai7GameMode* GetGameMode() const;

	void CheckAndShowMissionStatus();
};