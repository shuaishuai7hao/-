// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class ECharacterAnimState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Run UMETA(DisplayName = "Run"),
	Jump UMETA(DisplayName = "Jump"),
	JumpAir UMETA(DisplayName = "JumpAir"),
	Fall UMETA(DisplayName = "Fall"),
	Land UMETA(DisplayName = "Land"),
	Attack UMETA(DisplayName = "Attack")
};

UCLASS()
class SHUAI7_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	ACharacter* MyCharacterRef;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	UCharacterMovementComponent* CharacterMovementRef;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float VerticalVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	ECharacterAnimState CurrentAnimState;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsIdle;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsJumping;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsJumpAir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsLanding;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsOnGround;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsAttacking;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsCastingSkillE;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsCastingSkillQ;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsCastingSkillR;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsChargingOrb;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float OrbChargeProgress;

	UFUNCTION()
	void OnAttackHitNotify();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	ECharacterAnimState GetCurrentAnimState() const { return CurrentAnimState; }

protected:
	void UpdateAnimState();
};
