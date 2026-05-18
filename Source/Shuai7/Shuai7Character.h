// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Components/PrimitiveComponent.h"
#include "InputActionValue.h"
#include "NiagaraSystem.h"
#include "Shuai7Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionAnimationComponent;
class UAnimMontage;
class UParticleSystem;
class UInputMappingContext;
class UInputAction;
class UBoxComponent;
class USpringArmComponent;
class UUserWidget;
class USoundBase;
class UPrimitiveComponent;
class ADialogueNPC;
struct FInputActionValue;
struct FHitResult;

struct FNiagaraMovementState
{
	UNiagaraComponent* NiagaraComp;
	FVector ActorLoc;
	FVector Direction;
	float StartDist;
	float EndDist;
	float Height;
	float ElapsedTime;
	float Duration;

	FNiagaraMovementState(UNiagaraComponent* InComp, FVector InLoc, FVector InDir, float InStart, float InEnd, float InHeight, float InDuration)
		: NiagaraComp(InComp), ActorLoc(InLoc), Direction(InDir), StartDist(InStart), EndDist(InEnd), Height(InHeight), ElapsedTime(0.f), Duration(InDuration) {}
};

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Slime UMETA(DisplayName = "Slime"),
	Goblin UMETA(DisplayName = "Goblin"),
	Skeleton UMETA(DisplayName = "Skeleton")
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Patrol UMETA(DisplayName = "Patrol"),
	Chase UMETA(DisplayName = "Chase"),
	Attack UMETA(DisplayName = "Attack"),
	Hurt UMETA(DisplayName = "Hurt"),
	Death UMETA(DisplayName = "Death")
};

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Skill1 UMETA(DisplayName = "Skill1"),
	Skill2 UMETA(DisplayName = "Skill2"),
	Skill3 UMETA(DisplayName = "Skill3")
};

UENUM(BlueprintType)
enum class EUpgradeChoice : uint8
{
	HealthBoost UMETA(DisplayName = "HealthBoost"),
	DamageBoost UMETA(DisplayName = "DamageBoost"),
	SkillReward UMETA(DisplayName = "SkillReward")
};

UCLASS()
class SHUAI7_API AShuai7Character : public ACharacter
{
	GENERATED_BODY()

public:
	AShuai7Character();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SkillAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SpecialSkill1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SpecialSkill2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SpecialSkill3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShowSkillStatusAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PauseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat)
	UBoxComponent* AttackCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	TArray<UAnimMontage*> LightAttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	USoundBase* LightAttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Niagara")
	UNiagaraSystem* LightAttackNiagaraEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Niagara")
	FVector NiagaraAttachOffset = FVector(50.f, 0.f, 50.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	UAnimMontage* SkillAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	USoundBase* SkillAttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	UParticleSystem* SkillProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Skill Niagara")
	TArray<UNiagaraSystem*> SkillAttackNiagaraEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Skill Niagara")
	bool bUseCirclePattern = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Skill Niagara", meta = (ClampMin = "1", ClampMax = "16"))
	int32 SkillNiagaraCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Skill Niagara", meta = (ClampMin = "100", ClampMax = "1500"))
	float SkillNiagaraDistance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Skill Niagara", meta = (ClampMin = "0", ClampMax = "1500"))
	float SkillNiagaraStartDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Skill Niagara", meta = (ClampMin = "100", ClampMax = "2000"))
	float SkillNiagaraHeight = 800.f;

	TArray<FNiagaraMovementState> MovingNiagaras;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float ComboResetTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float AttackInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (ClampMin = "1", ClampMax = "100"))
	float LightAttackDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (ClampMin = "50", ClampMax = "500"))
	float LightAttackRange = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (ClampMin = "1", ClampMax = "1000"))
	float SkillAttackDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (ClampMin = "100", ClampMax = "2000"))
	float SkillAttackRange = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	USoundBase* SkillAttackVoice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (ClampMin = "0.1", ClampMax = "30.0"))
	float SkillAttackCooldown = 5.f;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	float SkillAttackCurrentCooldown = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	bool bIsAttacking;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	bool bIsChargingOrb;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	float OrbChargeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float MaxOrbChargeTime = 2.0f;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	int32 CurrentAttackStage;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	double LastAttackTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "1", ClampMax = "1000"))
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	bool bIsDead;

	bool bSkill2Immortal;

	bool bCanMove = true;

	UFUNCTION(BlueprintCallable, Category = "Skills")
	void DisableMovement(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Skills")
	void EnableMovement();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem")
	int32 Experience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem")
	int32 ExperienceToNextLevel = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem")
	int32 EnemiesKilled = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem")
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem")
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem")
	bool bIsWaitingForUpgrade = false;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem")
	int32 CurrentUpgradeChoiceCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem")
	TArray<ESkillType> AcquiredSkills;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem")
	float LifeStealPercent = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem")
	float DamageBoostPercent = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem")
	float HealthBoostAmount = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem")
	float DamageBoostPerStack = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem")
	float LifeStealPerStack = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UAnimMontage* Skill1Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UAnimMontage* Skill2Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UAnimMontage* Skill3Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UParticleSystem* Skill1Effect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UParticleSystem* Skill2Effect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2EffectDistance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UNiagaraSystem* Skill2Effect1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UNiagaraSystem* Skill2Effect2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UNiagaraSystem* Skill2Effect3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UNiagaraSystem* Skill2Effect4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UNiagaraSystem* Skill2Effect5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UNiagaraSystem* Skill2Effect6;

	// 技能2特效时间配置（开始时间和结束时间，单位：秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect1StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect1EndTime = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect2StartTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect2EndTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	FVector Skill2Effect2TargetOffset = FVector(140.f, 40.f, 140.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect2MoveEndTime = 4.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect2MoveStartTime = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Damage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2DamageInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2DamageRange = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2DamageWidth = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2DamageHeight = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	bool bSkill2ImmortalDuringCast = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	bool bSkill2CannotMoveDuringCast = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2CannotMoveDuration = 10.f;

	UFUNCTION(BlueprintCallable, Category = "LevelSystem|Skills")
	void ApplySkill2Damage();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect3StartTime = 1.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect3EndTime = 4.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect4StartTime = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect4EndTime = 4.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect5StartTime = 1.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect5EndTime = 4.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect6StartTime = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Effect6EndTime = 4.3f;

	// 技能2特效位置配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	FVector Skill2Effect1Offset = FVector(-80.f, -60.f, 100.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	FVector Skill2Effect2Offset = FVector(-80.f, 60.f, 100.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	FVector Skill2Effect3Offset = FVector(500.f, -10.f, 40.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	FVector Skill2Effect4Offset = FVector(500.f, 0.f, 40.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	FVector Skill2Effect5Offset = FVector(500.f, 10.f, 40.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	FVector Skill2Effect6Offset = FVector(500.f, 0.f, 40.f);

	// 技能2特效旋转配置（是否反转方向）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	bool Skill2Effect1FlipRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	bool Skill2Effect2FlipRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	bool Skill2Effect3FlipRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	bool Skill2Effect4FlipRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	bool Skill2Effect5FlipRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	bool Skill2Effect6FlipRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	UParticleSystem* Skill3Effect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	USoundBase* Skill1Voice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	USoundBase* Skill2Voice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	USoundBase* Skill3Voice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill1Damage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill3Damage = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill1Cooldown = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill2Cooldown = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill3Cooldown = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill3StunDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelSystem|Skills")
	float Skill3Radius = 2000.f;

	// 受击蒙太奇动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* HitMontage;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem|Skills")
	float Skill1CurrentCooldown = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem|Skills")
	float Skill2CurrentCooldown = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem|Skills")
	float Skill3CurrentCooldown = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem|Skills")
	bool bSkill1Unlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem|Skills")
	bool bSkill2Unlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem|Skills")
	bool bSkill3Unlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "LevelSystem|Skills")
	bool bIsDeathImmune = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> HealthBarClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	class UUserWidget* HealthBarWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> UpgradeWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	class UUserWidget* UpgradeWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> DamageNumberClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FVector DamageNumberOffset = FVector(0, 0, 100);

	UFUNCTION(BlueprintCallable, Category = Combat)
	void PerformLightAttack();

	UFUNCTION(BlueprintCallable, Category = Combat)
	void PerformSkillAttack();

	UFUNCTION(BlueprintCallable, Category = Combat)
	void PerformSkillAttackRaycast();

	UFUNCTION(BlueprintCallable, Category = Combat)
	void OnAttackAnimationEnd();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void Die();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	void AddExperience(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	void ChooseUpgrade(EUpgradeChoice Choice);

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	void OpenUpgradeMenu();

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	void CloseUpgradeMenu();

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	int32 GetRandomSkillReward();

	UFUNCTION(BlueprintCallable, Category = Combat)
	void SpawnSkill2Effects();

	FRotator GetEffectRotation(bool bFlipRotation);

	void SpawnAndDestroyEffect(UNiagaraSystem* Effect, FVector Location, FRotator Rotation, float Lifetime);

	void DestroyNiagaraComponent(UNiagaraComponent* NiagaraComp);

	struct FSkill2NiagaraMovementState
	{
		class UNiagaraComponent* NiagaraComp;
		FVector StartLocation;
		FVector EndLocation;
		float Duration;
		float ElapsedTime;
		float StartDelay;
	};

	TArray<FSkill2NiagaraMovementState> Skill2MovingNiagaras;

	void UpdateSkill2NiagaraMovement(float DeltaTime);

	void SpawnSkill2Effect2WithMove(UNiagaraSystem* Effect, FVector StartLoc, FVector TargetLoc, FRotator Rot, float Lifetime, float MoveStartDelay, float MoveDuration);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ResetCombo();

	UFUNCTION(BlueprintCallable, Category = "Skills")
	FString GetSkillStatusText();

	UFUNCTION(BlueprintCallable, Category = "Skills")
	bool IsSkillUnlocked(int32 SkillIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Skills")
	void HandleShowSkillStatus();

	UFUNCTION(BlueprintCallable, Category = "Skills")
	bool IsSkill1Unlocked() const { return bSkill1Unlocked; }

	UFUNCTION(BlueprintCallable, Category = "Skills")
	bool IsSkill2Unlocked() const { return bSkill2Unlocked; }

	UFUNCTION(BlueprintCallable, Category = "Skills")
	bool IsSkill3Unlocked() const { return bSkill3Unlocked; }

	UFUNCTION(BlueprintCallable, Category = "Skills")
	float GetSkill1CooldownRemaining() const { return Skill1CurrentCooldown; }

	UFUNCTION(BlueprintCallable, Category = "Skills")
	float GetSkill2CooldownRemaining() const { return Skill2CurrentCooldown; }

	UFUNCTION(BlueprintCallable, Category = "Skills")
	float GetSkill3CooldownRemaining() const { return Skill3CurrentCooldown; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetSkillAttackCooldownRemaining() const { return SkillAttackCurrentCooldown; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetSkillAttackCooldown() const { return SkillAttackCooldown; }

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	int32 GetScore() const { return Score; }

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	int32 GetKillCount() const { return KillCount; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	int32 GetExp() const { return Experience; }

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	int32 GetExpToNextLevel() const { return ExperienceToNextLevel; }

	UFUNCTION(BlueprintCallable, Category = "LevelSystem")
	int32 GetPlayerLevel() const { return Level; }

protected:
	UFUNCTION()
	void OnAttackCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void HandleLightAttack(const FInputActionValue& Value);
	void HandleSkillAttack(const FInputActionValue& Value);
	void HandleSpecialSkill1(const FInputActionValue& Value);
	void HandleSpecialSkill2(const FInputActionValue& Value);
	void HandleSpecialSkill3(const FInputActionValue& Value);
	void HandlePause(const FInputActionValue& Value);
	void HandleInteract(const FInputActionValue& Value);
	
protected:
	void PerformSpecialSkill(int32 SkillIndex);
	bool CanUseSkill(int32 SkillIndex) const;
	void ApplyLifeSteal(float DamageAmount);
	void UpdateHealthBar();
	void UpdateNiagaraMovement(float DeltaTime);
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
