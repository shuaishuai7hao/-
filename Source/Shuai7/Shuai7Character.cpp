// Copyright Epic Games, Inc. All Rights Reserved.

#include "Shuai7Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "EnemyCharacter.h"
#include "RangedEnemyCharacter.h"
#include "DialogueNPC.h"
#include "Shuai7GameMode.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "CollisionQueryParams.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Sound/SoundBase.h"
#include "Engine/EngineTypes.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "CollisionShape.h"
#include "Components/PrimitiveComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Logging/LogMacros.h"
#include "Collision.h"
#include "Engine/OverlapResult.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AShuai7Character::AShuai7Character()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	AttackCollision->SetupAttachment(GetMesh(), FName("hand_r"));
	AttackCollision->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	AttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AShuai7Character::OnAttackCollisionBeginOverlap);

	CurrentAttackStage = 0;
	LastAttackTime = 0.0;
	bIsAttacking = false;
	bIsChargingOrb = false;
	OrbChargeTime = 0.0f;

	CurrentHealth = MaxHealth;
	bIsDead = false;
	HealthBarWidget = nullptr;
	UpgradeWidgetInstance = nullptr;

	Level = 1;
	Experience = 0;
	EnemiesKilled = 0;
	ExperienceToNextLevel = 3;
	bIsWaitingForUpgrade = false;
	CurrentUpgradeChoiceCount = 0;
	LifeStealPercent = 0.f;
	DamageBoostPercent = 0.f;
	bSkill1Unlocked = false;
	bSkill2Unlocked = false;
	bSkill3Unlocked = false;
	Skill1CurrentCooldown = 0.f;
	Skill2CurrentCooldown = 0.f;
	Skill3CurrentCooldown = 0.f;
}

void AShuai7Character::BeginPlay()
{
	Super::BeginPlay();

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

void AShuai7Character::UpdateHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(bIsDead ? ESlateVisibility::Hidden : ESlateVisibility::Visible);

		UProgressBar* HealthBar = Cast<UProgressBar>(HealthBarWidget->GetWidgetFromName(TEXT("HealthBar")));
		if (HealthBar)
		{
			HealthBar->SetPercent(GetHealthPercent());
		}

		UTextBlock* HealthText = Cast<UTextBlock>(HealthBarWidget->GetWidgetFromName(TEXT("HealthText")));
		if (HealthText)
		{
			HealthText->SetText(FText::Format(NSLOCTEXT("Shuai7", "HealthFormat", "{0}/{1}"),
				FText::AsNumber((int32)CurrentHealth),
				FText::AsNumber((int32)MaxHealth)));
		}
	}
}

void AShuai7Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShuai7Character::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShuai7Character::Look);

		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AShuai7Character::HandleLightAttack);

		EnhancedInputComponent->BindAction(SkillAttackAction, ETriggerEvent::Started, this, &AShuai7Character::HandleSkillAttack);

		EnhancedInputComponent->BindAction(SpecialSkill1Action, ETriggerEvent::Started, this, &AShuai7Character::HandleSpecialSkill1);
		EnhancedInputComponent->BindAction(SpecialSkill2Action, ETriggerEvent::Started, this, &AShuai7Character::HandleSpecialSkill2);
		EnhancedInputComponent->BindAction(SpecialSkill3Action, ETriggerEvent::Started, this, &AShuai7Character::HandleSpecialSkill3);
		EnhancedInputComponent->BindAction(ShowSkillStatusAction, ETriggerEvent::Started, this, &AShuai7Character::HandleShowSkillStatus);
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AShuai7Character::HandlePause);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AShuai7Character::HandleInteract);
	}
}

void AShuai7Character::Move(const FInputActionValue& Value)
{
	if (!bCanMove)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AShuai7Character::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AShuai7Character::HandleLightAttack(const FInputActionValue& Value)
{
	if (bIsWaitingForUpgrade)
	{
		return;
	}

	double CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastAttackTime > AttackInterval)
	{
		PerformLightAttack();
	}
}

void AShuai7Character::HandleSkillAttack(const FInputActionValue& Value)
{
	if (bIsWaitingForUpgrade)
	{
		return;
	}

	if (!bIsAttacking)
	{
		PerformSkillAttack();
	}
}

void AShuai7Character::HandleSpecialSkill1(const FInputActionValue& Value)
{
	// 技能1是被动技能，不能主动释放
	// 当生命值归零时会自动触发死亡免疫效果
}

void AShuai7Character::HandleSpecialSkill2(const FInputActionValue& Value)
{
	if (bIsWaitingForUpgrade)
	{
		return;
	}

	PerformSpecialSkill(2);
}

void AShuai7Character::HandleSpecialSkill3(const FInputActionValue& Value)
{
	if (bIsWaitingForUpgrade)
	{
		return;
	}

	PerformSpecialSkill(3);
}

void AShuai7Character::PerformLightAttack()
{
	if (bIsAttacking)
	{
		return;
	}

	if (!GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	if (LightAttackMontages.Num() == 0)
	{
		return;
	}

	double CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastAttackTime > ComboResetTime)
	{
		CurrentAttackStage = 0;
	}

	int32 MontageIndex = CurrentAttackStage % LightAttackMontages.Num();

	if (UAnimMontage* AttackMontage = LightAttackMontages[MontageIndex])
	{
		bIsAttacking = true;

		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->SetMovementMode(MOVE_None);

		PlayAnimMontage(AttackMontage);

		if (LightAttackNiagaraEffect)
		{
			FVector SpawnLocation = GetActorLocation() - GetActorForwardVector() * FMath::Abs(NiagaraAttachOffset.X) + FVector(0.f, 0.f, NiagaraAttachOffset.Z);
			FRotator SpawnRotation = GetActorRotation();
			SpawnRotation.Yaw += 180.f;
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LightAttackNiagaraEffect, SpawnLocation, SpawnRotation);
		}

		// 播放轻攻击音效
		if (LightAttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), LightAttackSound, GetActorLocation());
		}

		if (AttackCollision)
		{
			AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}

		CurrentAttackStage++;
		LastAttackTime = CurrentTime;
	}
}

void AShuai7Character::PerformSkillAttack()
{
	if (bIsAttacking)
	{
		return;
	}

	if (!GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	if (SkillAttackCurrentCooldown > 0.f)
	{
		return;
	}

	if (SkillAttackMontage)
	{
		bIsAttacking = true;
		SkillAttackCurrentCooldown = SkillAttackCooldown;

		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->SetMovementMode(MOVE_None);

		PlayAnimMontage(SkillAttackMontage);

		// 播放技能攻击音效
		if (SkillAttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SkillAttackSound, GetActorLocation());
		}

		if (SkillAttackVoice)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SkillAttackVoice, GetActorLocation());
		}

		FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.0f;
		FRotator SpawnRotation = GetActorRotation();

		if (SkillProjectile)
		{
			UParticleSystemComponent* Projectile = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				SkillProjectile,
				SpawnLocation,
				SpawnRotation,
				true
			);

			if (Projectile)
			{
				Projectile->SetVectorParameter(FName("Direction"), GetActorForwardVector());
				Projectile->Activate(true);
			}
		}

		if (SkillAttackNiagaraEffects.Num() > 0)
		{
			if (bUseCirclePattern)
			{
				FVector Forward = GetActorForwardVector();
				FVector Right = GetActorRightVector();
				FVector Up = FVector(0.f, 0.f, 1.f);
				FVector ActorLoc = GetActorLocation();
				float StartDist = SkillNiagaraStartDistance;
				float EndDist = SkillNiagaraDistance;
				float Height = SkillNiagaraHeight;
				float Duration = 2.5f;

				for (int32 i = 0; i < SkillNiagaraCount; i++)
				{
					float Angle = (360.f / SkillNiagaraCount) * i;
					float Radian = FMath::DegreesToRadians(Angle);
					
					FVector Direction = Forward * FMath::Cos(Radian) + Right * FMath::Sin(Radian);
					Direction.Normalize();
					
					FVector StartOffset = Direction * StartDist;
					FVector StartLocation = ActorLoc + StartOffset + Up * Height;

					UNiagaraSystem* EffectToUse = SkillAttackNiagaraEffects[i % SkillAttackNiagaraEffects.Num()];
					if (EffectToUse)
					{
						UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EffectToUse, StartLocation, SpawnRotation);
						if (NiagaraComp)
						{
							NiagaraComp->SetAutoDestroy(true);
							
							FNiagaraMovementState* State = new FNiagaraMovementState(NiagaraComp, ActorLoc, Direction, StartDist, EndDist, Height, Duration);
							MovingNiagaras.Add(*State);
							delete State;
						}
					}
				}

			}
			else
			{
				FVector Forward = GetActorForwardVector();
				FVector Right = GetActorRightVector();

				for (int32 i = 0; i < SkillNiagaraCount; i++)
				{
					float OffsetY = (i - (SkillNiagaraCount - 1) / 2.0f) * 80.f;
					FVector EffectLocation = GetActorLocation() + Forward * SkillNiagaraDistance + Right * OffsetY + FVector(0.f, 0.f, SkillNiagaraHeight);

					UNiagaraSystem* EffectToUse = SkillAttackNiagaraEffects[i % SkillAttackNiagaraEffects.Num()];
					if (EffectToUse)
					{
						UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EffectToUse, EffectLocation, SpawnRotation);
						if (NiagaraComp)
						{
							NiagaraComp->SetAutoDestroy(true);
						}
					}
				}
			}

			PerformSkillAttackRaycast();
		}
	}
}

void AShuai7Character::PerformSkillAttackRaycast()
{
	FVector CenterLocation = GetActorLocation();
	float ActualDamage = SkillAttackDamage * (1.f + DamageBoostPercent / 100.f);

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (!Actor || Actor == this)
		{
			continue;
		}

		float Distance = FVector::Distance(CenterLocation, Actor->GetActorLocation());
		if (Distance > SkillAttackRange)
		{
			continue;
		}

		if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Actor))
		{
			if (!Enemy->bIsDead)
			{
				Enemy->TakeDamage(ActualDamage);
				ApplyLifeSteal(ActualDamage);

				if (DamageNumberClass)
				{
					UUserWidget* DamageWidget = CreateWidget<UUserWidget>(GetWorld(), DamageNumberClass);
					if (DamageWidget)
					{
						DamageWidget->AddToViewport();
						FVector2D ScreenPosition;
						FVector WorldPosition = Enemy->GetActorLocation() + DamageNumberOffset;
						UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), WorldPosition, ScreenPosition);
						DamageWidget->SetPositionInViewport(ScreenPosition);
					}
				}
			}
			continue;
		}

		if (class ARangedEnemyCharacter* RangedEnemy = Cast<class ARangedEnemyCharacter>(Actor))
		{
			if (!RangedEnemy->bIsDead)
			{
				RangedEnemy->TakeDamage(ActualDamage);
				ApplyLifeSteal(ActualDamage);

				if (DamageNumberClass)
				{
					UUserWidget* DamageWidget = CreateWidget<UUserWidget>(GetWorld(), DamageNumberClass);
					if (DamageWidget)
					{
						DamageWidget->AddToViewport();
						FVector2D ScreenPosition;
						FVector WorldPosition = RangedEnemy->GetActorLocation() + DamageNumberOffset;
						UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), WorldPosition, ScreenPosition);
						DamageWidget->SetPositionInViewport(ScreenPosition);
					}
				}
			}
		}
	}
}

void AShuai7Character::PerformSpecialSkill(int32 SkillIndex)
{
	if (SkillIndex == 1 && bSkill1Unlocked && CanUseSkill(1))
	{
		bIsAttacking = true;
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->SetMovementMode(MOVE_None);

		if (Skill1Montage)
		{
			PlayAnimMontage(Skill1Montage);
		}

		if (Skill1Effect)
		{
			FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.f;
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Skill1Effect, SpawnLocation, GetActorRotation());
		}

		Skill1CurrentCooldown = Skill1Cooldown;

		FVector StartLocation = GetActorLocation();
		FVector EndLocation = StartLocation + GetActorForwardVector() * 500.f;

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
		{
			float ActualDamage = Skill1Damage * (1.f + DamageBoostPercent / 100.f);

			if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitResult.GetActor()))
			{
				if (!Enemy->bIsDead)
				{
					Enemy->TakeDamage(ActualDamage);
					ApplyLifeSteal(ActualDamage);
				}
			}

			if (class ARangedEnemyCharacter* RangedEnemy = Cast<class ARangedEnemyCharacter>(HitResult.GetActor()))
			{
				if (!RangedEnemy->bIsDead)
				{
					RangedEnemy->TakeDamage(ActualDamage);
					ApplyLifeSteal(ActualDamage);
				}
			}
		}

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AShuai7Character::OnAttackAnimationEnd, 0.5f, false);
	}
	else if (SkillIndex == 2 && bSkill2Unlocked && CanUseSkill(2))
	{
		bIsAttacking = true;
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->SetMovementMode(MOVE_None);

		if (Skill2Montage)
		{
			PlayAnimMontage(Skill2Montage);
		}

		if (Skill2Effect)
		{
			FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.f;
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Skill2Effect, SpawnLocation, GetActorRotation());
		}

		Skill2CurrentCooldown = Skill2Cooldown;

		FVector StartLocation = GetActorLocation();
		FVector EndLocation = StartLocation + GetActorForwardVector() * Skill2EffectDistance;

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn, QueryParams))
		{
			float ActualDamage = Skill2Damage * (1.f + DamageBoostPercent / 100.f);

			if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitResult.GetActor()))
			{
				if (!Enemy->bIsDead)
				{
					Enemy->TakeDamage(ActualDamage);
					ApplyLifeSteal(ActualDamage);
				}
			}

			if (class ARangedEnemyCharacter* RangedEnemy = Cast<class ARangedEnemyCharacter>(HitResult.GetActor()))
			{
				if (!RangedEnemy->bIsDead)
				{
					RangedEnemy->TakeDamage(ActualDamage);
					ApplyLifeSteal(ActualDamage);
				}
			}
		}

		// 添加技能2的Niagara特效组合（按顺序出现）
		SpawnSkill2Effects();

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AShuai7Character::OnAttackAnimationEnd, 0.5f, false);
	}
	else if (SkillIndex == 3 && bSkill3Unlocked && CanUseSkill(3))
	{
		bIsAttacking = true;
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->SetMovementMode(MOVE_None);

		if (Skill3Montage)
		{
			PlayAnimMontage(Skill3Montage);
		}

		if (Skill3Effect)
		{
			FVector SpawnLocation = GetActorLocation();
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Skill3Effect, SpawnLocation, FRotator::ZeroRotator);
		}
		if (Skill3Voice)
		{
			UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), Skill3Voice);
			if (AudioComponent)
			{
				AudioComponent->SetVolumeMultiplier(3.0f);
				AudioComponent->Play();
			}
		}
		Skill3CurrentCooldown = Skill3Cooldown;

		TArray<FOverlapResult> Overlaps;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = false;

		// 使用更广泛的碰撞检测
		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

		bool bSuccess = GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity,
			ObjectParams, FCollisionShape::MakeSphere(Skill3Radius), QueryParams);

		for (FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (!HitActor) continue;

			if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
			{
				if (!Enemy->bIsDead)
				{
					Enemy->Stun(Skill3StunDuration);
				}
			}

			if (class ARangedEnemyCharacter* RangedEnemy = Cast<class ARangedEnemyCharacter>(HitActor))
			{
				if (!RangedEnemy->bIsDead)
				{
					RangedEnemy->Stun(Skill3StunDuration);
				}
			}
		}

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AShuai7Character::OnAttackAnimationEnd, 0.5f, false);
	}
}

bool AShuai7Character::CanUseSkill(int32 SkillIndex) const
{
	if (SkillIndex == 1)
	{
		return Skill1CurrentCooldown <= 0.f;
	}
	if (SkillIndex == 2)
	{
		return Skill2CurrentCooldown <= 0.f;
	}
	if (SkillIndex == 3)
	{
		return Skill3CurrentCooldown <= 0.f;
	}
	return false;
}

void AShuai7Character::ResetCombo()
{
	bIsAttacking = false;
}

void AShuai7Character::OnAttackAnimationEnd()
{
	bIsAttacking = false;

	if (AttackCollision)
	{
		AttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AShuai7Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 确保活着的时候可以移动
	if (!bIsDead && GetCharacterMovement()->MovementMode == MOVE_None && !bIsAttacking)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	if (bIsAttacking)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && !AnimInstance->Montage_IsPlaying(nullptr))
		{
			OnAttackAnimationEnd();
		}
	}

	if (Skill1CurrentCooldown > 0.f)
	{
		Skill1CurrentCooldown -= DeltaTime;
	}
	else if (bIsDeathImmune)
	{
		bIsDeathImmune = false;
	}
	if (Skill2CurrentCooldown > 0.f)
	{
		Skill2CurrentCooldown -= DeltaTime;
	}
	if (Skill3CurrentCooldown > 0.f)
	{
		Skill3CurrentCooldown -= DeltaTime;
	}
	if (SkillAttackCurrentCooldown > 0.f)
	{
		SkillAttackCurrentCooldown -= DeltaTime;
	}

	UpdateNiagaraMovement(DeltaTime);
	UpdateSkill2NiagaraMovement(DeltaTime);
}

void AShuai7Character::OnAttackCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsAttacking || !OtherActor || OtherActor == this)
	{
		return;
	}

	float ActualDamage = LightAttackDamage * (1.f + DamageBoostPercent / 100.f);

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(OtherActor);
	if (Enemy && !Enemy->bIsDead)
	{
		Enemy->TakeDamage(ActualDamage);
		ApplyLifeSteal(ActualDamage);

		if (DamageNumberClass)
		{
			UUserWidget* DamageWidget = CreateWidget<UUserWidget>(GetWorld(), DamageNumberClass);
			if (DamageWidget)
			{
				DamageWidget->AddToViewport();
				FVector2D ScreenPosition;
				FVector WorldPosition = Enemy->GetActorLocation() + DamageNumberOffset;
				UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), WorldPosition, ScreenPosition);
				DamageWidget->SetPositionInViewport(ScreenPosition);
			}
		}
		return;
	}

	class ARangedEnemyCharacter* RangedEnemy = Cast<class ARangedEnemyCharacter>(OtherActor);
	if (RangedEnemy && !RangedEnemy->bIsDead)
	{
		RangedEnemy->TakeDamage(ActualDamage);
		ApplyLifeSteal(ActualDamage);

		if (DamageNumberClass)
		{
			UUserWidget* DamageWidget = CreateWidget<UUserWidget>(GetWorld(), DamageNumberClass);
			if (DamageWidget)
			{
				DamageWidget->AddToViewport();
				FVector2D ScreenPosition;
				FVector WorldPosition = RangedEnemy->GetActorLocation() + DamageNumberOffset;
				UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), WorldPosition, ScreenPosition);
				DamageWidget->SetPositionInViewport(ScreenPosition);
			}
		}
	}
}

void AShuai7Character::ApplyLifeSteal(float DamageAmount)
{
	if (LifeStealPercent > 0.f)
	{
		float HealAmount = DamageAmount * (LifeStealPercent / 100.f);
		Heal(HealAmount);
	}
}

void AShuai7Character::TakeDamage(float DamageAmount)
{
	if (bIsDead)
	{
		return;
	}

	if (bSkill2Immortal)
	{
		return;
	}

	CurrentHealth -= DamageAmount;

	// 播放受击蒙太奇动画
	if (HitMontage && !bIsDead)
	{
		PlayAnimMontage(HitMontage);
	}

	if (CurrentHealth <= 0.f)
	{
		CurrentHealth = 0.f;

		if (bSkill1Unlocked && Skill1CurrentCooldown <= 0.f && !bIsDeathImmune)
		{
			bIsDeathImmune = true;
			CurrentHealth = MaxHealth;
			Skill1CurrentCooldown = Skill1Cooldown;

			if (Skill1Effect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Skill1Effect, GetActorLocation(), GetActorRotation());
			}

			if (Skill1Voice)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), Skill1Voice, GetActorLocation());
			}

			UpdateHealthBar();
			return;
		}

		Die();
	}

	UpdateHealthBar();
}

void AShuai7Character::Heal(float HealAmount)
{
	if (bIsDead)
	{
		return;
	}

	CurrentHealth += HealAmount;

	if (CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}

	UpdateHealthBar();
}

void AShuai7Character::Die()
{
	bIsDead = true;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_None);

	bIsAttacking = false;

	UpdateHealthBar();

	if (GetWorld())
	{
		class AShuai7GameMode* GameMode = Cast<class AShuai7GameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->OnPlayerDeath();
		}
	}
}

float AShuai7Character::GetHealthPercent() const
{
	if (MaxHealth <= 0.f)
	{
		return 0.f;
	}

	return CurrentHealth / MaxHealth;
}

void AShuai7Character::AddExperience(int32 Amount)
{
	if (bIsDead)
	{
		return;
	}

	Experience += Amount;
	EnemiesKilled++;

	if (Experience >= ExperienceToNextLevel)
	{
		Experience -= ExperienceToNextLevel;
		Level++;
		bIsWaitingForUpgrade = true;
		CurrentUpgradeChoiceCount = 0;
		OpenUpgradeMenu();
	}
}

void AShuai7Character::OpenUpgradeMenu()
{
	if (UpgradeWidgetClass && !UpgradeWidgetInstance)
	{
		UpgradeWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), UpgradeWidgetClass);
		if (UpgradeWidgetInstance)
		{
			UpgradeWidgetInstance->AddToViewport();

			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->SetInputMode(FInputModeUIOnly());
				PC->bShowMouseCursor = true;
			}

			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				if (UpgradeWidgetInstance)
				{
					UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);
				}
			}, 0.5f, false);
		}
	}
}

void AShuai7Character::CloseUpgradeMenu()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	if (UpgradeWidgetInstance)
	{
		UpgradeWidgetInstance->RemoveFromParent();
		UpgradeWidgetInstance = nullptr;

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
	bIsWaitingForUpgrade = false;
}

void AShuai7Character::ChooseUpgrade(EUpgradeChoice Choice)
{
	CurrentUpgradeChoiceCount++;

	if (Choice == EUpgradeChoice::HealthBoost)
	{
		MaxHealth += HealthBoostAmount;
		CurrentHealth = MaxHealth;
		UpdateHealthBar();
		CloseUpgradeMenu();
	}
	else if (Choice == EUpgradeChoice::DamageBoost)
	{
		DamageBoostPercent += DamageBoostPerStack;
		LifeStealPercent += LifeStealPerStack * 100.f;
		CloseUpgradeMenu();
	}
	else if (Choice == EUpgradeChoice::SkillReward)
	{
		if (CurrentUpgradeChoiceCount >= 3)
		{
			int32 RandomChoice = FMath::RandRange(1, 2);
			if (RandomChoice == 1)
			{
				ChooseUpgrade(EUpgradeChoice::HealthBoost);
				return;
			}
			else
			{
				ChooseUpgrade(EUpgradeChoice::DamageBoost);
				return;
			}
		}

		int32 SkillIndex = GetRandomSkillReward();

		if (SkillIndex == 1 && !bSkill1Unlocked)
		{
			bSkill1Unlocked = true;
			AcquiredSkills.Add(ESkillType::Skill1);
		}
		else if (SkillIndex == 2 && !bSkill2Unlocked)
		{
			bSkill2Unlocked = true;
			AcquiredSkills.Add(ESkillType::Skill2);
		}
		else if (SkillIndex == 3 && !bSkill3Unlocked)
		{
			bSkill3Unlocked = true;
			AcquiredSkills.Add(ESkillType::Skill3);
		}
		else
		{
			int32 RandomChoice = FMath::RandRange(1, 2);
			if (RandomChoice == 1)
			{
				ChooseUpgrade(EUpgradeChoice::HealthBoost);
				return;
			}
			else
			{
				ChooseUpgrade(EUpgradeChoice::DamageBoost);
				return;
			}
		}

		CloseUpgradeMenu();
	}
}

int32 AShuai7Character::GetRandomSkillReward()
{
	TArray<int32> AvailableSkills;

	if (!bSkill1Unlocked)
	{
		AvailableSkills.Add(1);
	}
	if (!bSkill2Unlocked)
	{
		AvailableSkills.Add(2);
	}
	if (!bSkill3Unlocked)
	{
		AvailableSkills.Add(3);
	}

	if (AvailableSkills.Num() == 0)
	{
		return 0;
	}

	int32 RandomIndex = FMath::RandRange(0, AvailableSkills.Num() - 1);
	return AvailableSkills[RandomIndex];
}

FString AShuai7Character::GetSkillStatusText()
{
	FString StatusText = TEXT("技能状态:\n");
	
	StatusText += FString::Printf(TEXT("[Q] 技能1: %s\n"), bSkill1Unlocked ? TEXT("已解锁") : TEXT("未解锁"));
	StatusText += FString::Printf(TEXT("[Shift] 技能2: %s\n"), bSkill2Unlocked ? TEXT("已解锁") : TEXT("未解锁"));
	StatusText += FString::Printf(TEXT("[R] 技能3: %s\n"), bSkill3Unlocked ? TEXT("已解锁") : TEXT("未解锁"));
	
	return StatusText;
}

bool AShuai7Character::IsSkillUnlocked(int32 SkillIndex) const
{
	switch (SkillIndex)
	{
	case 1:
		return bSkill1Unlocked;
	case 2:
		return bSkill2Unlocked;
	case 3:
		return bSkill3Unlocked;
	default:
		return false;
	}
}

void AShuai7Character::HandleShowSkillStatus()
{
}

void AShuai7Character::HandlePause(const FInputActionValue& Value)
{
	if (GetWorld())
	{
		class AShuai7GameMode* GameMode = Cast<class AShuai7GameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->ShowPauseMenu();
		}
	}
}

void AShuai7Character::HandleInteract(const FInputActionValue& Value)
{
	FVector PlayerLocation = GetActorLocation();
	float InteractionRadius = 350.f;

	TArray<AActor*> AllNPCs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADialogueNPC::StaticClass(), AllNPCs);

	for (AActor* Actor : AllNPCs)
	{
		class ADialogueNPC* NPC = Cast<class ADialogueNPC>(Actor);
		if (NPC)
		{
			float Distance = FVector::Distance(PlayerLocation, NPC->GetActorLocation());
			if (Distance <= InteractionRadius)
			{
				NPC->OnPlayerInteract();
				return;
			}
		}
	}
}

void AShuai7Character::UpdateNiagaraMovement(float DeltaTime)
{
	for (int32 i = MovingNiagaras.Num() - 1; i >= 0; i--)
	{
		FNiagaraMovementState& State = MovingNiagaras[i];
		
		if (!State.NiagaraComp)
		{
			MovingNiagaras.RemoveAt(i);
			continue;
		}

		State.ElapsedTime += DeltaTime;
		float Progress = FMath::Clamp(State.ElapsedTime / State.Duration, 0.f, 1.f);
		float CurrentDistance = FMath::Lerp(State.StartDist, State.EndDist, Progress);
		FVector CurrentOffset = State.Direction * CurrentDistance;
		FVector NewLocation = State.ActorLoc + CurrentOffset + FVector(0.f, 0.f, 1.f) * State.Height;
		State.NiagaraComp->SetWorldLocation(NewLocation);

		if (Progress >= 1.f)
		{
			State.NiagaraComp->DeactivateImmediate();
			State.NiagaraComp->DestroyComponent();
			MovingNiagaras.RemoveAt(i);
		}
	}
}

void AShuai7Character::UpdateSkill2NiagaraMovement(float DeltaTime)
{
	for (int32 i = Skill2MovingNiagaras.Num() - 1; i >= 0; i--)
	{
		FSkill2NiagaraMovementState& State = Skill2MovingNiagaras[i];
		
		if (!State.NiagaraComp)
		{
			Skill2MovingNiagaras.RemoveAt(i);
			continue;
		}

		State.ElapsedTime += DeltaTime;
		
		// 只有当ElapsedTime >= 0时才开始移动
		if (State.ElapsedTime < 0)
		{
			continue;
		}
		
		float Progress = FMath::Clamp(State.ElapsedTime / State.Duration, 0.f, 1.f);
		FVector NewLocation = FMath::Lerp(State.StartLocation, State.EndLocation, Progress);
		State.NiagaraComp->SetWorldLocation(NewLocation);

		if (Progress >= 1.f)
		{
			Skill2MovingNiagaras.RemoveAt(i);
		}
	}
}

void AShuai7Character::SpawnSkill2Effect2WithMove(UNiagaraSystem* Effect, FVector StartLoc, FVector TargetLoc, FRotator Rot, float Lifetime, float MoveStartDelay, float MoveDuration)
{
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, StartLoc, Rot);
	if (NiagaraComp)
	{
		if (Lifetime > 0)
		{
			NiagaraComp->SetAutoDestroy(false);
			FTimerHandle TimerHandleEnd;
			GetWorld()->GetTimerManager().SetTimer(TimerHandleEnd, FTimerDelegate::CreateUObject(this, &AShuai7Character::DestroyNiagaraComponent, NiagaraComp), Lifetime, false);
		}
		else
		{
			NiagaraComp->SetAutoDestroy(true);
		}
		
		// 添加到移动列表，StartDelay控制移动开始前的等待时间
		FSkill2NiagaraMovementState MoveState;
		MoveState.NiagaraComp = NiagaraComp;
		MoveState.StartLocation = StartLoc;
		MoveState.EndLocation = TargetLoc;
		MoveState.Duration = MoveDuration;
		MoveState.ElapsedTime = -MoveStartDelay; // 负值表示等待时间
		MoveState.StartDelay = MoveStartDelay;
		Skill2MovingNiagaras.Add(MoveState);
	}
}

FRotator AShuai7Character::GetEffectRotation(bool bFlipRotation)
{
	FRotator Rot = GetActorRotation();
	if (bFlipRotation)
	{
		Rot.Yaw += 180.f;
	}
	return Rot;
}

void AShuai7Character::SpawnSkill2Effects()
{
	// 播放技能2语音
	if (Skill2Voice)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Skill2Voice, GetActorLocation());
	}

	FVector ActorLoc = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();
	FVector Up = GetActorUpVector();

	// NS_01
	if (Skill2Effect1)
	{
		FVector Loc = ActorLoc + Skill2Effect1Offset.X * Forward + Skill2Effect1Offset.Y * Right + Skill2Effect1Offset.Z * Up;
		FRotator Rot = this->GetEffectRotation(Skill2Effect1FlipRotation);
		float Delay1 = Skill2Effect1StartTime;
		float Lifetime1 = Skill2Effect1EndTime - Skill2Effect1StartTime;
		
		FTimerHandle TimerHandle1;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle1, FTimerDelegate::CreateUObject(this, &AShuai7Character::SpawnAndDestroyEffect, Skill2Effect1, Loc, Rot, Lifetime1), Delay1, false);
	}

	// NS_02 - 带有移动功能
	if (Skill2Effect2)
	{
		FVector Loc2 = ActorLoc + Skill2Effect2Offset.X * Forward + Skill2Effect2Offset.Y * Right + Skill2Effect2Offset.Z * Up;
		FVector TargetLoc2 = ActorLoc + Skill2Effect2TargetOffset.X * Forward + Skill2Effect2TargetOffset.Y * Right + Skill2Effect2TargetOffset.Z * Up;
		FRotator Rot2 = this->GetEffectRotation(Skill2Effect2FlipRotation);
		float Delay2 = Skill2Effect2StartTime;
		float Lifetime2 = Skill2Effect2EndTime - Skill2Effect2StartTime;
		float MoveStart2 = Skill2Effect2MoveStartTime - Skill2Effect2StartTime;
		float MoveDuration2 = Skill2Effect2MoveEndTime - Skill2Effect2MoveStartTime;
		
		// 使用成员函数委托
		FTimerHandle TimerHandle2;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle2, FTimerDelegate::CreateUObject(this, &AShuai7Character::SpawnSkill2Effect2WithMove, Skill2Effect2, Loc2, TargetLoc2, Rot2, Lifetime2, MoveStart2, MoveDuration2), Delay2, false);
	}

	// NS_03
	if (Skill2Effect3)
	{
		FVector Loc = ActorLoc + Skill2Effect3Offset.X * Forward + Skill2Effect3Offset.Y * Right + Skill2Effect3Offset.Z * Up;
		FRotator Rot = this->GetEffectRotation(Skill2Effect3FlipRotation);
		float Delay3 = Skill2Effect3StartTime;
		float Lifetime3 = Skill2Effect3EndTime - Skill2Effect3StartTime;
		
		FTimerHandle TimerHandle3;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle3, FTimerDelegate::CreateUObject(this, &AShuai7Character::SpawnAndDestroyEffect, Skill2Effect3, Loc, Rot, Lifetime3), Delay3, false);
	}

	// NS_04
	if (Skill2Effect4)
	{
		FVector Loc = ActorLoc + Skill2Effect4Offset.X * Forward + Skill2Effect4Offset.Y * Right + Skill2Effect4Offset.Z * Up;
		FRotator Rot = this->GetEffectRotation(Skill2Effect4FlipRotation);
		float Delay4 = Skill2Effect4StartTime;
		float Lifetime4 = Skill2Effect4EndTime - Skill2Effect4StartTime;
		
		FTimerHandle TimerHandle4;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle4, FTimerDelegate::CreateUObject(this, &AShuai7Character::SpawnAndDestroyEffect, Skill2Effect4, Loc, Rot, Lifetime4), Delay4, false);
	}

	// NS_05
	if (Skill2Effect5)
	{
		FVector Loc = ActorLoc + Skill2Effect5Offset.X * Forward + Skill2Effect5Offset.Y * Right + Skill2Effect5Offset.Z * Up;
		FRotator Rot = this->GetEffectRotation(Skill2Effect5FlipRotation);
		float Delay5 = Skill2Effect5StartTime;
		float Lifetime5 = Skill2Effect5EndTime - Skill2Effect5StartTime;
		
		FTimerHandle TimerHandle5;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle5, FTimerDelegate::CreateUObject(this, &AShuai7Character::SpawnAndDestroyEffect, Skill2Effect5, Loc, Rot, Lifetime5), Delay5, false);
	}

	// NS_06
	if (Skill2Effect6)
	{
		FVector Loc = ActorLoc + Skill2Effect6Offset.X * Forward + Skill2Effect6Offset.Y * Right + Skill2Effect6Offset.Z * Up;
		FRotator Rot = this->GetEffectRotation(Skill2Effect6FlipRotation);
		float Delay6 = Skill2Effect6StartTime;
		float Lifetime6 = Skill2Effect6EndTime - Skill2Effect6StartTime;
		
		FTimerHandle TimerHandle6;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle6, FTimerDelegate::CreateUObject(this, &AShuai7Character::SpawnAndDestroyEffect, Skill2Effect6, Loc, Rot, Lifetime6), Delay6, false);
	}

	// 角色无法移动状态
	if (bSkill2CannotMoveDuringCast)
	{
		DisableMovement(Skill2CannotMoveDuration);
	}

	// 角色无法受伤状态
	if (bSkill2ImmortalDuringCast)
	{
		bSkill2Immortal = true;
		FTimerHandle ImmortalTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(ImmortalTimerHandle, FTimerDelegate::CreateLambda([this]()
		{
			bSkill2Immortal = false;
		}), Skill2CannotMoveDuration, false);
	}

	// 伤害启动延迟（前1.7秒不造成伤害）
	float DamageStartDelay = 1.7f;
	
	// 伤害结束时间（从技能释放开始计算，总共10秒）
	float DamageEndTime = 10.f;
	
	// 启动伤害定时器 - 延迟1.7秒后开始，每0.1秒造成一次伤害
	FTimerHandle* DamageTimerHandle = new FTimerHandle();
	GetWorld()->GetTimerManager().SetTimer(*DamageTimerHandle, FTimerDelegate::CreateUObject(this, &AShuai7Character::ApplySkill2Damage), Skill2DamageInterval, true, DamageStartDelay);

	// 伤害实际持续时间 = 结束时间 - 启动延迟
	float ActualDamageDuration = DamageEndTime - DamageStartDelay;
	if (ActualDamageDuration > 0.f)
	{
		// 10秒后停止伤害（从技能释放开始计算）
		FTimerHandle StopDamageTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(StopDamageTimerHandle, FTimerDelegate::CreateLambda([this, DamageTimerHandle]()
		{
			GetWorld()->GetTimerManager().ClearTimer(*DamageTimerHandle);
			delete DamageTimerHandle;
		}), DamageEndTime, false);
	}
}

void AShuai7Character::ApplySkill2Damage()
{
	FVector ActorLoc = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();
	FVector Up = GetActorUpVector();

	FVector BoxCenter = ActorLoc + Forward * (Skill2DamageRange / 2.f);
	FVector BoxHalfExtents = FVector(Skill2DamageRange / 2.f, Skill2DamageWidth / 2.f, Skill2DamageHeight / 2.f);

	FQuat BoxRotation = FQuat(GetActorRotation());
	FCollisionShape BoxShape;
	BoxShape.SetBox(FVector3f(BoxHalfExtents.X, BoxHalfExtents.Y, BoxHalfExtents.Z));

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FOverlapResult> Overlaps;
	bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(Overlaps, BoxCenter, BoxRotation, ECC_Pawn, BoxShape, QueryParams);

	if (bHasOverlaps)
	{
		for (FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (HitActor && HitActor != this)
			{
				// 检测近战敌人
				AEnemyCharacter* MeleeEnemy = Cast<AEnemyCharacter>(HitActor);
				if (MeleeEnemy)
				{
					MeleeEnemy->TakeDamage(Skill2Damage);
					continue;
				}

				// 检测远程敌人
				ARangedEnemyCharacter* RangedEnemy = Cast<ARangedEnemyCharacter>(HitActor);
				if (RangedEnemy)
				{
					RangedEnemy->TakeDamage(Skill2Damage);
				}
			}
		}
	}
}

void AShuai7Character::DisableMovement(float Duration)
{
	bCanMove = false;
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &AShuai7Character::EnableMovement), Duration, false);
}

void AShuai7Character::EnableMovement()
{
	bCanMove = true;
}

void AShuai7Character::SpawnAndDestroyEffect(UNiagaraSystem* Effect, FVector Location, FRotator Rotation, float Lifetime)
{
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, Location, Rotation);
	if (NiagaraComp)
	{
		if (Lifetime > 0)
		{
			NiagaraComp->SetAutoDestroy(false);
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &AShuai7Character::DestroyNiagaraComponent, NiagaraComp), Lifetime, false);
		}
		else
		{
			NiagaraComp->SetAutoDestroy(true);
		}
	}
}

void AShuai7Character::DestroyNiagaraComponent(UNiagaraComponent* NiagaraComp)
{
	if (NiagaraComp)
	{
		NiagaraComp->DeactivateImmediate();
		NiagaraComp->DestroyComponent();
	}
}


