#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RangedEnemyCharacter.generated.h"

class UAnimMontage;
class UParticleSystem;
class UUserWidget;
class USoundBase;

UCLASS()
class SHUAI7_API ARangedEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARangedEnemyCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "1", ClampMax = "2000"))
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	bool bIsDead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MeleeDamage = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SkillDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MeleeAttackRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SkillAttackRange = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SkillCooldown = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* SkillMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundBase* SkillSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MaxAudioDistance = 1000.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsHurt;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsSkillReady;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	bool bIsActivated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ChaseRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ReturnToPatrolRange = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UParticleSystem* HurtEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UParticleSystem* DeathEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UParticleSystem* SkillEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UParticleSystem* ProjectileEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* HurtMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* DeathMontage;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 ExperienceReward = 1;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AttackPlayer();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnAttackAnimationEnd();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Stun(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	bool IsWithinAudioRange() const;

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void AdjustToGround();

protected:	UFUNCTION(BlueprintCallable, Category = "AI")
	void FacePlayer();

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnEffect(UParticleSystem* Effect, FVector Offset = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBar();

private:
	class AShuai7Character* PlayerRef;

	FVector PatrolOrigin;
	FVector CurrentPatrolTarget;

	double LastAttackTime;
	double LastHurtTime;
	double LastPatrolTime;
	double SkillReadyTime;

	bool bIsMeleeAttack;

	bool bIsStunned;
	float StunRemainingTime;

	FTimerHandle TimerHandle_Destroy;
	FTimerHandle TimerHandle_WaitForLoad;

	void UpdateAI(float DeltaTime);
	void HandlePatrol();
	FVector GetRandomNavPatrolPoint() const;
	void MoveToLocation(FVector TargetLocation);
	void UpdateHealthBarPosition();
	void SpawnProjectile();
	void DestroyEnemy();
	void CheckPlayerVisibility();
};
