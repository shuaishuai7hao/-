// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UAnimMontage;
class UParticleSystem;
class UUserWidget;

UCLASS()
class SHUAI7_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 50.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	bool bIsDead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackRange = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackInterval = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* HurtMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UParticleSystem* AttackEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UParticleSystem* HurtEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UParticleSystem* DeathEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ChaseRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ReturnToPatrolRange = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HealthBarClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UUserWidget* HealthBarWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FVector HealthBarOffset = FVector(0.f, 0.f, 150.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float MaxHealthBarDisplayDistance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ActivationDistance = 2000.f;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsActivated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 ExperienceReward = 1;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnAttackAnimationEnd();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnHurtAnimationEnd();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Stun(float Duration);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateAI(float DeltaTime);

private:
	FVector PatrolOrigin;
	FVector CurrentPatrolTarget;
	float LastAttackTime;
	float LastHurtTime;
	float LastPatrolTime;
	class AShuai7Character* PlayerRef;
	FTimerHandle TimerHandle_Destroy;

	bool bIsStunned;
	float StunRemainingTime;

	bool IsPlayerInAttackRange() const;
	bool IsPlayerInChaseRange() const;
	bool IsFarFromPatrolOrigin() const;
	FVector GetRandomNavPatrolPoint() const;
	void MoveToLocation(FVector TargetLocation);
	void FacePlayer();
	void SpawnEffect(UParticleSystem* Effect, FVector Offset = FVector::ZeroVector);
	void DestroyEnemy();
	void HandlePatrol();
	void UpdateHealthBar();
	void UpdateHealthBarPosition();
	void CheckPlayerVisibility();
};
