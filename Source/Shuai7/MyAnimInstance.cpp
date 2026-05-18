// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Shuai7Character.h"
#include "Math/UnrealMathUtility.h"

void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	MyCharacterRef = Cast<ACharacter>(TryGetPawnOwner());
	if (MyCharacterRef)
	{
		CharacterMovementRef = MyCharacterRef->GetCharacterMovement();
	}

	bIsCastingSkillE = false;
	bIsCastingSkillQ = false;
	bIsCastingSkillR = false;
	bIsChargingOrb = false;
	OrbChargeProgress = 0.f;
	
	CurrentAnimState = ECharacterAnimState::Idle;
	bIsIdle = true;
	bIsRunning = false;
	bIsJumping = false;
	bIsJumpAir = false;
	bIsFalling = false;
	bIsLanding = false;
	bIsMoving = false;
	bIsOnGround = true;
	bIsAttacking = false;
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (MyCharacterRef && CharacterMovementRef)
	{
		Speed = MyCharacterRef->GetVelocity().Size2D();
		VerticalVelocity = MyCharacterRef->GetVelocity().Z;
		
		bIsOnGround = CharacterMovementRef->IsMovingOnGround();
		bIsMoving = Speed > 5.f;
		
		if (bIsOnGround)
		{
			if (Speed < 10.f)
			{
				bIsIdle = true;
				bIsRunning = false;
			}
			else
			{
				bIsIdle = false;
				bIsRunning = true;
			}
			
			bIsJumping = false;
			bIsJumpAir = false;
			bIsFalling = false;
			bIsLanding = false;
		}
		else
		{
			bIsIdle = false;
			bIsRunning = false;
			
			if (VerticalVelocity > 50.f)
			{
				bIsJumping = true;
				bIsJumpAir = false;
				bIsFalling = false;
				bIsLanding = false;
			}
			else if (FMath::Abs(VerticalVelocity) <= 50.f)
			{
				bIsJumping = false;
				bIsJumpAir = true;
				bIsFalling = false;
				bIsLanding = false;
			}
			else
			{
				bIsJumping = false;
				bIsJumpAir = false;
				bIsFalling = true;
				bIsLanding = false;
			}
		}
	}

	if (AShuai7Character* Shuai7Char = Cast<AShuai7Character>(TryGetPawnOwner()))
	{
		bIsAttacking = Shuai7Char->bIsAttacking;
		bIsChargingOrb = Shuai7Char->bIsChargingOrb;
		
		if (Shuai7Char->MaxOrbChargeTime > 0.f)
		{
			OrbChargeProgress = Shuai7Char->OrbChargeTime / Shuai7Char->MaxOrbChargeTime;
		}
		else
		{
			OrbChargeProgress = 0.f;
		}
	}

	UpdateAnimState();
}

void UMyAnimInstance::UpdateAnimState()
{
	if (bIsAttacking)
	{
		CurrentAnimState = ECharacterAnimState::Attack;
	}
	else if (bIsJumping)
	{
		CurrentAnimState = ECharacterAnimState::Jump;
	}
	else if (bIsJumpAir)
	{
		CurrentAnimState = ECharacterAnimState::JumpAir;
	}
	else if (bIsFalling)
	{
		CurrentAnimState = ECharacterAnimState::Fall;
	}
	else if (bIsLanding)
	{
		CurrentAnimState = ECharacterAnimState::Land;
	}
	else if (bIsRunning)
	{
		CurrentAnimState = ECharacterAnimState::Run;
	}
	else
	{
		CurrentAnimState = ECharacterAnimState::Idle;
	}
}

void UMyAnimInstance::OnAttackHitNotify()
{
	if (AShuai7Character* Shuai7Char = Cast<AShuai7Character>(TryGetPawnOwner()))
	{
		Shuai7Char->OnAttackAnimationEnd();
	}
}
