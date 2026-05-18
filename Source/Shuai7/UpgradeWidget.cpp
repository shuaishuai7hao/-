#include "UpgradeWidget.h"
#include "Shuai7Character.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerCharacter = Cast<AShuai7Character>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if (Button_HealthBoost)
	{
		Button_HealthBoost->OnClicked.AddDynamic(this, &UUpgradeWidget::OnHealthBoostClicked);
	}

	if (Button_DamageBoost)
	{
		Button_DamageBoost->OnClicked.AddDynamic(this, &UUpgradeWidget::OnDamageBoostClicked);
	}

	if (Button_SkillReward)
	{
		Button_SkillReward->OnClicked.AddDynamic(this, &UUpgradeWidget::OnSkillRewardClicked);
	}

	UpdateTexts();
}

void UUpgradeWidget::UpdateTexts()
{
	if (Text_Level)
	{
		Text_Level->SetText(FText::FromString(TEXT("Level Up!")));
	}

	if (Text_HealthBoost)
	{
		Text_HealthBoost->SetText(FText::FromString(TEXT("钢化你心\n提高生命值")));
	}

	if (Text_DamageBoost)
	{
		Text_DamageBoost->SetText(FText::FromString(TEXT("吸血习性\n提高攻击力\n并获得吸血")));
	}

	if (Text_SkillReward)
	{
		Text_SkillReward->SetText(FText::FromString(TEXT("质变：棱彩阶\n随机解锁一个\n强大的技能\n(q,r,shift)")));
	}
}

void UUpgradeWidget::OnHealthBoostClicked()
{
	if (PlayerCharacter)
	{
		PlayerCharacter->ChooseUpgrade(EUpgradeChoice::HealthBoost);
	}
}

void UUpgradeWidget::OnDamageBoostClicked()
{
	if (PlayerCharacter)
	{
		PlayerCharacter->ChooseUpgrade(EUpgradeChoice::DamageBoost);
	}
}

void UUpgradeWidget::OnSkillRewardClicked()
{
	if (PlayerCharacter)
	{
		PlayerCharacter->ChooseUpgrade(EUpgradeChoice::SkillReward);
	}
}
