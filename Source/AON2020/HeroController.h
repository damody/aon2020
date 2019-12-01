// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Hero.h"
#include "HeroController.generated.h"

/**
 * 
 */
UCLASS()
class AON2020_API AHeroController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;
	// End PlayerController interface

	uint32 bMoveToMouseCursor : 1;
	uint32 bMouseRButton : 1;
	uint32 bMouseLButton : 1;
	uint32 bLeftShiftDown : 1;
	uint32 bRightShiftDown : 1;
	UPrimitiveComponent* ClickedPrimitive;

public:
	AHeroController();

	// 有註冊的鍵盤事件
	TMap<FKey, EKeyBehavior> KeyMapping;

	// 本地玩家的英雄
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AHero* LocalHero;

	// 當前滑鼠坐標
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D CurrentMouseXY;

	FVector2D GetMouseScreenPosition();

	void OnMouseLDown();
	void OnMouseRDown();

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerCharacterMove(class AUnit* hero, const FVector& pos);

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "MOBA")
	void ServerAttackCompute(AUnit* attacker, AUnit* victim, EDamageType dtype,
		float damage, bool AttackLanded);
};
