// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyCharacter.h"
#include "Shuai7Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->MaxWalkSpeed = 200.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 100.f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	GetCharacterMovement()->bRequestedMoveUseAcceleration = true;
	GetCharacterMovement()->MaxAcceleration = 500.f;

	CurrentHealth = MaxHealth;
	bIsDead = false;
	LastAttackTime = 0.f;
	LastHurtTime = 0.f;
	LastPatrolTime = 0.f;
	CurrentPatrolTarget = FVector::ZeroVector;
	HealthBarWidget = nullptr;
	bIsStunned = false;
	StunRemainingTime = 0.f;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

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

	CurrentHealth = MaxHealth;
}

void AEnemyCharacter::Tick(float DeltaTime)
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
}

void AEnemyCharacter::UpdateAI(float DeltaTime)
{
	if (!PlayerRef || PlayerRef->bIsDead)
	{
		HandlePatrol();
		return;
	}

	float DistanceToPlayer = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());

	if (DistanceToPlayer <= AttackRange)
	{
		FRotator LookAtRotation = (PlayerRef->GetActorLocation() - GetActorLocation()).Rotation();
		LookAtRotation.Pitch = 0.f;
		LookAtRotation.Roll = 0.f;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookAtRotation, DeltaTime, 10.f));

		double CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastAttackTime > AttackInterval)
		{
			PerformAttack();
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

void AEnemyCharacter::UpdateHealthBar()
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
			HealthText->SetText(FText::Format(NSLOCTEXT("Shuai7", "EnemyHealthFormat", "{0}/{1}"),
				FText::AsNumber((int32)CurrentHealth),
				FText::AsNumber((int32)MaxHealth)));
		}
	}
}

void AEnemyCharacter::UpdateHealthBarPosition()
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

void AEnemyCharacter::HandlePatrol()
{
	double CurrentTime = GetWorld()->GetTimeSeconds();

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

FVector AEnemyCharacter::GetRandomNavPatrolPoint() const
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

bool AEnemyCharacter::IsPlayerInAttackRange() const
{
	if (!PlayerRef)
	{
		return false;
	}

	float Distance = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());
	return Distance <= AttackRange;
}

bool AEnemyCharacter::IsPlayerInChaseRange() const
{
	if (!PlayerRef)
	{
		return false;
	}

	float Distance = FVector::Distance(GetActorLocation(), PlayerRef->GetActorLocation());
	return Distance <= ChaseRange;
}

bool AEnemyCharacter::IsFarFromPatrolOrigin() const
{
	float Distance = FVector::Distance(GetActorLocation(), PatrolOrigin);
	return Distance > PatrolRadius + 100.f;
}

void AEnemyCharacter::PerformAttack()
{
	if (!PlayerRef)
	{
		return;
	}

	LastAttackTime = GetWorld()->GetTimeSeconds();

	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}

	SpawnEffect(AttackEffect, FVector(0, 0, 50));
	PlayerRef->TakeDamage(AttackDamage);
}

void AEnemyCharacter::OnAttackAnimationEnd()
{
}

void AEnemyCharacter::OnHurtAnimationEnd()
{
}

void AEnemyCharacter::Stun(float Duration)
{
	if (bIsDead)
	{
		return;
	}

	bIsStunned = true;
	StunRemainingTime = Duration;
	GetCharacterMovement()->StopMovementImmediately();
}

void AEnemyCharacter::SpawnEffect(UParticleSystem* Effect, FVector Offset)
{
	if (!Effect)
	{
		return;
	}

	FVector SpawnLocation = GetActorLocation() + Offset;
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Effect, SpawnLocation);
}

void AEnemyCharacter::TakeDamage(float DamageAmount)
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

		if (PlayerRef)
		{
			PlayerRef->AddExperience(ExperienceReward);
		}

		if (DeathMontage)
		{
			PlayAnimMontage(DeathMontage);
		}

		SpawnEffect(DeathEffect, FVector(0, 0, 50));

		float DestroyDelay = DeathMontage ? FMath::Max(0.1f, DeathMontage->GetPlayLength() - 0.5f) : 0.5f;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Destroy, this, &AEnemyCharacter::DestroyEnemy, DestroyDelay, false);
	}
	else
	{
		if (HurtMontage)
		{
			PlayAnimMontage(HurtMontage);
		}

		SpawnEffect(HurtEffect, FVector(0, 0, 50));
	}

	UpdateHealthBar();
}

void AEnemyCharacter::DestroyEnemy()
{
	Destroy();
}

void AEnemyCharacter::CheckPlayerVisibility()
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
