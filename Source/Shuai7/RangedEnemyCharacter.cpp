#include "RangedEnemyCharacter.h"
#include "Shuai7Character.h"
#include "Shuai7GameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

ARangedEnemyCharacter::ARangedEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->MaxWalkSpeed = 200.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 100.f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	GetCharacterMovement()->bRequestedMoveUseAcceleration = true;
	GetCharacterMovement()->MaxAcceleration = 500.f;

	bIsDead = false;
	bIsAttacking = false;
	bIsHurt = false;
	bIsSkillReady = true;
	bIsMeleeAttack = false;
	LastAttackTime = 0.f;
	LastHurtTime = 0.f;
	LastPatrolTime = 0.f;
	SkillReadyTime = 0.f;
	CurrentPatrolTarget = FVector::ZeroVector;
	HealthBarWidget = nullptr;
	bIsStunned = false;
	StunRemainingTime = 0.f;
}

void ARangedEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 在BeginPlay中初始化当前生命值（此时蓝图属性已应用）
	CurrentHealth = MaxHealth;

	// 暂停敌人移动，等待玩家视野检测
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	bIsActivated = false;

	PatrolOrigin = GetActorLocation();
	PlayerRef = Cast<AShuai7Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (HealthBarClass && !HealthBarWidget)
	{
		HealthBarWidget = CreateWidget<UUserWidget>(GetWorld(), HealthBarClass);
		if (HealthBarWidget)
		{
			HealthBarWidget->AddToViewport();
			UpdateHealthBar();
		}
	}
}

void ARangedEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead)
	{
		return;
	}

	// 如果敌人未激活，检查玩家是否能看到敌人
	if (!bIsActivated)
	{
		CheckPlayerVisibility();
		// 未激活时隐藏血条
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		return; // 未激活时不执行其他逻辑
	}

	if (bIsStunned)
	{
		StunRemainingTime -= DeltaTime;
		if (StunRemainingTime <= 0.f)
		{
			bIsStunned = false;
			StunRemainingTime = 0.f;
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
		else
		{
			GetCharacterMovement()->StopMovementImmediately();
			UpdateHealthBarPosition();
			return;
		}
	}

	if (HealthBarWidget && PlayerRef)
	{
		float DistanceToPlayer = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());

		if (DistanceToPlayer > MaxHealthBarDisplayDistance)
		{
			HealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			HealthBarWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}

	UpdateAI(DeltaTime);
	UpdateHealthBarPosition();

	if (bIsAttacking)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (!AnimInstance->Montage_IsPlaying(nullptr))
			{
				OnAttackAnimationEnd();
			}
		}
	}

	if (bIsHurt)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (!AnimInstance->Montage_IsPlaying(nullptr))
			{
				bIsHurt = false;
			}
		}
	}
}

void ARangedEnemyCharacter::UpdateAI(float DeltaTime)
{
	if (!PlayerRef || PlayerRef->bIsDead)
	{
		HandlePatrol();
		return;
	}

	float DistanceToPlayer = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());

	if (DistanceToPlayer <= MeleeAttackRange)
	{
		FRotator LookAtRotation = (PlayerRef->GetActorLocation() - GetActorLocation()).Rotation();
		LookAtRotation.Pitch = 0.f;
		LookAtRotation.Roll = 0.f;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookAtRotation, DeltaTime, 10.f));

		double CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastAttackTime > AttackInterval)
		{
			AttackPlayer();
		}
	}
	else if (DistanceToPlayer <= ChaseRange)
	{
		FVector Direction = PlayerRef->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.f;
		Direction.Normalize();
		AddMovementInput(Direction, 1.f);
	}
	else
	{
		HandlePatrol();
	}
}

void ARangedEnemyCharacter::HandlePatrol()
{
	double CurrentTime = GetWorld()->GetTimeSeconds();

	if (PatrolRadius <= 0.f)
	{
		return;
	}

	if (CurrentTime - LastPatrolTime > 0.5f)
	{
		float DistanceToTarget = CurrentPatrolTarget.IsZero() ? FLT_MAX :
			FVector::Distance(GetActorLocation(), CurrentPatrolTarget);

		if (DistanceToTarget < 30.f || CurrentPatrolTarget.IsZero())
		{
			CurrentPatrolTarget = GetRandomNavPatrolPoint();
			LastPatrolTime = CurrentTime;
		}
	}

	if (!CurrentPatrolTarget.IsZero())
	{
		FVector Direction = CurrentPatrolTarget - GetActorLocation();
		Direction.Z = 0.f;
		float Distance = Direction.Size();

		if (Distance > 30.f)
		{
			Direction.Normalize();
			AddMovementInput(Direction, 1.f);
		}
	}
}

FVector ARangedEnemyCharacter::GetRandomNavPatrolPoint() const
{
	if (PatrolRadius <= 0.f)
	{
		return PatrolOrigin;
	}

	float RandomAngle = FMath::RandRange(0.f, 360.f);
	float RandomDistance = FMath::RandRange(100.f, PatrolRadius);

	FVector TargetPoint = PatrolOrigin + FVector(
		FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
		FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
		0.f
	);

	FHitResult Hit;
	FVector TraceStart = FVector(TargetPoint.X, TargetPoint.Y, TargetPoint.Z + 500.f);
	FVector TraceEnd = FVector(TargetPoint.X, TargetPoint.Y, TargetPoint.Z - 500.f);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility);

	if (Hit.bBlockingHit)
	{
		TargetPoint.Z = Hit.ImpactPoint.Z;
	}

	return TargetPoint;
}

void ARangedEnemyCharacter::AttackPlayer()
{
	double CurrentTime = GetWorld()->GetTimeSeconds();

	if (bIsAttacking || CurrentTime - LastAttackTime < AttackInterval)
	{
		return;
	}

	if (!PlayerRef) return;

	float DistanceToPlayer = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());

	bool bUseSkill = bIsSkillReady && DistanceToPlayer > MeleeAttackRange && DistanceToPlayer <= SkillAttackRange;

	if (bUseSkill)
	{
		bIsAttacking = true;
		bIsMeleeAttack = false;
		LastAttackTime = CurrentTime;
		bIsSkillReady = false;
		SkillReadyTime = CurrentTime + SkillCooldown;

		if (SkillMontage)
		{
			PlayAnimMontage(SkillMontage);
		}

		// 播放技能攻击音效（检查距离）
		if (SkillSound && IsWithinAudioRange())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SkillSound, GetActorLocation());
		}

		SpawnEffect(SkillEffect, FVector(0, 0, 50));
		SpawnProjectile();
	}
	else if (DistanceToPlayer <= MeleeAttackRange)
	{
		bIsAttacking = true;
		bIsMeleeAttack = true;
		LastAttackTime = CurrentTime;

		if (AttackMontage)
		{
			PlayAnimMontage(AttackMontage);
		}
		else
		{
			PlayerRef->TakeDamage(MeleeDamage);
			bIsAttacking = false;
		}

		// 播放普通攻击音效（检查距离）
		if (AttackSound && IsWithinAudioRange())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetActorLocation());
		}
	}

	if (!bIsSkillReady && CurrentTime >= SkillReadyTime)
	{
		bIsSkillReady = true;
	}
}

void ARangedEnemyCharacter::OnAttackAnimationEnd()
{
	bIsAttacking = false;

	if (bIsMeleeAttack && PlayerRef && !PlayerRef->bIsDead)
	{
		float DistanceToPlayer = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());
		if (DistanceToPlayer <= MeleeAttackRange)
		{
			PlayerRef->TakeDamage(MeleeDamage);
			SpawnEffect(HurtEffect, FVector(0, 0, 50));
		}
	}
}

void ARangedEnemyCharacter::Stun(float Duration)
{
	if (bIsDead)
	{
		return;
	}

	bIsStunned = true;
	StunRemainingTime = Duration;
	GetCharacterMovement()->StopMovementImmediately();

	bIsAttacking = false;
	bIsHurt = false;
}

void ARangedEnemyCharacter::SpawnProjectile()
{
	if (!PlayerRef || !ProjectileEffect) return;

	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.f + FVector(0, 0, 80.f);
	FRotator SpawnRotation = (PlayerRef->GetActorLocation() - SpawnLocation).Rotation();

	UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ProjectileEffect, SpawnLocation, SpawnRotation);
	if (ParticleComp)
	{
		ParticleComp->SetVectorParameter("TargetLocation", PlayerRef->GetActorLocation() + FVector(0, 0, 50.f));
		ParticleComp->Activate(true);
	}

	FTimerHandle TimerHandle_ProjectileHit;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ProjectileHit, [this]()
	{
		if (PlayerRef && !PlayerRef->bIsDead)
		{
			PlayerRef->TakeDamage(SkillDamage);
			SpawnEffect(HurtEffect, FVector(0, 0, 50));
		}
	}, 0.5f, false);
}

void ARangedEnemyCharacter::SpawnEffect(UParticleSystem* Effect, FVector Offset)
{
	if (!Effect) return;

	FVector SpawnLocation = GetActorLocation() + Offset;
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Effect, SpawnLocation);
}

void ARangedEnemyCharacter::TakeDamage(float DamageAmount)
{
	if (bIsDead)
	{
		return;
	}

	CurrentHealth -= DamageAmount;

	if (CurrentHealth <= 0.f)
	{
		CurrentHealth = 0.f;
		bIsDead = true;

		GetCharacterMovement()->StopMovementImmediately();

		bIsAttacking = false;
		bIsHurt = false;

		if (PlayerRef)
		{
			PlayerRef->AddExperience(ExperienceReward);
		}

		if (DeathMontage)
		{
			PlayAnimMontage(DeathMontage);
		}

		// 播放死亡音效（检查距离）
		if (DeathSound && IsWithinAudioRange())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
		}

		SpawnEffect(DeathEffect, FVector(0, 0, 50));

		// 通知任务系统敌人被杀
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld());
		if (GameMode)
		{
			AShuai7GameMode* MissionGameMode = Cast<AShuai7GameMode>(GameMode);
			if (MissionGameMode)
			{
				MissionGameMode->OnEnemyKilled();
			}
		}

		float DestroyDelay = DeathMontage ? DeathMontage->GetPlayLength() : 0.5f;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Destroy, this, &ARangedEnemyCharacter::DestroyEnemy, DestroyDelay, false);
	}
	else
	{
		if (!bIsAttacking && HurtMontage)
		{
			bIsHurt = true;
			GetCharacterMovement()->StopMovementImmediately();
			PlayAnimMontage(HurtMontage);
		}

		SpawnEffect(HurtEffect, FVector(0, 0, 50));
	}

	UpdateHealthBar();
}

void ARangedEnemyCharacter::UpdateHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(bIsDead ? ESlateVisibility::Hidden : ESlateVisibility::Visible);

		UProgressBar* HealthBar = Cast<UProgressBar>(HealthBarWidget->GetWidgetFromName(TEXT("HealthBar")));
		if (HealthBar)
		{
			float HealthPercent = MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;
			HealthBar->SetPercent(HealthPercent);
		}

		UTextBlock* HealthText = Cast<UTextBlock>(HealthBarWidget->GetWidgetFromName(TEXT("HealthText")));
		if (HealthText)
		{
			HealthText->SetText(FText::Format(NSLOCTEXT("Shuai7", "RangedEnemyHealth", "{0}/{1}"),
				FText::AsNumber((int32)CurrentHealth),
				FText::AsNumber((int32)MaxHealth)));
		}
	}
}

void ARangedEnemyCharacter::FacePlayer()
{
	if (!PlayerRef) return;

	FRotator LookAtRotation = (PlayerRef->GetActorLocation() - GetActorLocation()).Rotation();
	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Roll = 0.f;
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), 10.f));
}

void ARangedEnemyCharacter::UpdateHealthBarPosition()
{
	if (HealthBarWidget && !bIsDead)
	{
		FVector2D ScreenPosition;
		FVector WorldPosition = GetActorLocation() + HealthBarOffset;
		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), WorldPosition, ScreenPosition);

		FVector2D WidgetSize = HealthBarWidget->GetDesiredSize();
		ScreenPosition.X -= WidgetSize.X / 2.0f;

		HealthBarWidget->SetPositionInViewport(ScreenPosition);
	}
}

bool ARangedEnemyCharacter::IsWithinAudioRange() const
{
	if (PlayerRef)
	{
		float Distance = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());
		return Distance <= MaxAudioDistance;
	}
	return true;
}

void ARangedEnemyCharacter::DestroyEnemy()
{
	Destroy();
}

void ARangedEnemyCharacter::AdjustToGround()
{
	// 获取胶囊体组件
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule) return;

	// 射线检测参数 - 使用WorldStatic来检测地面
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	Params.AddIgnoredActor(this);

	// 获取胶囊体尺寸
	float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	float CapsuleRadius = Capsule->GetScaledCapsuleRadius();

	// 保存原始位置
	FVector OriginalLocation = GetActorLocation();
	FVector CapsuleBottom = OriginalLocation - FVector(0, 0, CapsuleHalfHeight);

	// 向下检测（使用ECC_WorldStatic channel更准确）
	FVector Start = CapsuleBottom;
	FVector End = Start - FVector(0, 0, 500.f);

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
	{
		// 找到了地面
		float DistanceToGround = FVector::Distance(CapsuleBottom, Hit.Location);
		if (DistanceToGround > 5.f) // 只有距离超过5单位才调整
		{
			FVector NewLocation = Hit.Location + FVector(0, 0, CapsuleHalfHeight);
			SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
	else
	{
		// 向下没找到，尝试更远的距离
		End = Start - FVector(0, 0, 2000.f);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
		{
			FVector NewLocation = Hit.Location + FVector(0, 0, CapsuleHalfHeight);
			SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		}
		// 如果还是找不到，就保持原位置不变
	}
}

void ARangedEnemyCharacter::CheckPlayerVisibility()
{
	if (!PlayerRef)
	{
		PlayerRef = Cast<AShuai7Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		return;
	}

	// 简单距离检测激活
	float Distance = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());
	if (Distance <= ActivationDistance)
	{
		// 玩家在激活距离内，激活敌人
		bIsActivated = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}