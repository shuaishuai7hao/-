// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DialogueNPC.generated.h"

class UUserWidget;
class UButton;
class UAnimMontage;
class USoundBase;

UCLASS()
class SHUAI7_API ADialogueNPC : public ACharacter
{
	GENERATED_BODY()

public:
	ADialogueNPC();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TSubclassOf<UUserWidget> DialogueWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	UUserWidget* DialogueWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FVector InteractionWidgetOffset = FVector(0.f, 0.f, 200.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* InteractionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float InteractionSoundVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UAnimMontage* DialogueMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* DialogueSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MaxAudioDistance = 500.f;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	bool IsWithinAudioRange() const;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bIsPlayerNearby = false;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void HideDialogue();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void OnPlayerInteract();

private:
	UPROPERTY()
	class AShuai7Character* PlayerRef;

	UPROPERTY()
	UAudioComponent* ActiveDialogueAudio;

	UPROPERTY()
	bool bHasPlayedDialogueSound = false;

	UPROPERTY()
	bool bHasCompletedDialogue = false;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionRadius = 200.f;

	void UpdatePlayerProximity();
};