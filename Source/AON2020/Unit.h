﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeroAction.h"
#include "Unit.generated.h"


UENUM(BlueprintType)
enum class EShowMethod : uint8
{
	SEQUENCE UMETA(DisplayName = "順序播放"),
	RANDOM UMETA(DisplayName = "隨機播放"),
	SHOW_END UMETA(Hidden, DisplayName = ""),
};


UCLASS()
class AON2020_API AUnit : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, WithValidation, Reliable)
	void PushAction(FHeroAction action);

	UFUNCTION(Server, WithValidation, Reliable)
	void OverideAction(FHeroAction action);
	
	void DoAction(const FHeroAction& CurrentAction);

	//停止所有動作
	void DoNothing();

	//做移動到指定位置
	void DoAction_MoveToPosition(const FHeroAction& CurrentAction);

	//確定當前動作做完了沒
	bool CheckCurrentActionFinish();

	//推出做完的動作
	void PopAction();

	//依序做完裡面的動作
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current")
	TArray<FHeroAction> ActionQueue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current")
	FHeroAction CurrentAction;
	
	UPROPERTY(EditAnywhere, meta = (DisplayName = "攻擊順序"), Category = "aon")
	EShowMethod AttackShow;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "攻擊動作"), Category = "aon")
	TArray<UAnimMontage*> Attacks;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "被打動作"), Category = "aon")
	UAnimMontage* BeAttack;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "暈眩動作"), Category = "aon")
	UAnimMontage* Stun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current", Replicated)
	EHeroBodyStatus BodyStatus;

	//是否活著
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA|Current", Replicated)
	bool IsAlive = true;

	//最後一次移動的位置
	FVector LastMoveTarget = FVector::ZeroVector;

	uint16 Seq = 0;

};
