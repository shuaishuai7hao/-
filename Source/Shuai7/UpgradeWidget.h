#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class SHUAI7_API UUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_HealthBoost;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_DamageBoost;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_SkillReward;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* Text_HealthBoost;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* Text_DamageBoost;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* Text_SkillReward;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* Text_Level;

protected:
	virtual void NativeConstruct() override;

	void UpdateTexts();

	UFUNCTION()
	void OnHealthBoostClicked();

	UFUNCTION()
	void OnDamageBoostClicked();

	UFUNCTION()
	void OnSkillRewardClicked();

private:
	class AShuai7Character* PlayerCharacter;
};
