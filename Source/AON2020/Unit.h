// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AONAbilitySystemComponent.h"
#include "HeroAction.h"
#include "Unit.generated.h"


UENUM(BlueprintType)
enum class EShowMethod : uint8
{
	SEQUENCE UMETA(DisplayName = "順序播放"),
	RANDOM UMETA(DisplayName = "隨機播放"),
	SHOW_END UMETA(Hidden, DisplayName = ""),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChangeDelegate, float, HP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMPChangeDelegate, float, MP, float, MaxMP);

class UWebInterfaceJsonValue;
class UWebInterfaceJsonObject;
class UParticleSystemComponent;

UCLASS()
class AON2020_API AUnit : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UAONAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystem;
	};

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// tick裡的動作檢查迴圈
	void ActionLoop();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, WithValidation, Reliable)
	void PushAction(FHeroAction action);

	UFUNCTION(Server, WithValidation, Reliable)
	void OverideAction(FHeroAction action);
	
	
	void OverideAction2(FHeroAction action);
	
	UFUNCTION(NetMulticast, WithValidation, Reliable)
	void Montage_Play(UAnimMontage* MontageToPlay, float InPlayRate = 1.f);
	
	
	void DoAction(const FHeroAction& CurrentAction);

	//停止所有動作
	void DoNothing();

	//做移動到指定位置
	void DoAction_MoveToPosition(const FHeroAction& CurrentAction);

	//確定當前動作做完了沒
	bool CheckCurrentActionFinish();

	//推出做完的動作
	void PopAction();

	//產生動作
	UWebInterfaceJsonObject* BuildJsonObject();

	UFUNCTION()
	void OnHPChange(float HP, float MaxHP);
	UFUNCTION()
	void OnMPChange(float MP, float MaxMP);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "血量變動事件OnHPChange"), Category = "CF1")
	void BP_OnHPChange(float HP, float MaxHP);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "魔力變動事件OnMPChange"), Category = "CF1")
	void BP_OnMPChange(float MP, float MaxMP);
	
	UPROPERTY(BlueprintReadOnly)
	class UAONAbilitySystemComponent* AbilitySystem;

	//英雄名/單位名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon")
	FString UnitName;

	//隊伍id
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA|Current")
	int32 TeamId;

	UPROPERTY(EditAnywhere, Category = "aon", meta = (DisplayName = "血量"), ReplicatedUsing = OnRep_HP)
	int16 HP = 1000;

	UPROPERTY(EditAnywhere, Category = "aon", meta = (DisplayName = "最大血量"), ReplicatedUsing = OnRep_MaxHP)
	int16 MaxHP = 1000;

	UPROPERTY(EditAnywhere, Category = "aon", meta = (DisplayName = "魔力"), ReplicatedUsing = OnRep_MP)
	int16 MP = 1000;
	
	UPROPERTY(EditAnywhere, Category = "aon", meta = (DisplayName = "最大魔力"), ReplicatedUsing = OnRep_MaxMP)
	int16 MaxMP = 1000;

	//攻擊距離
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon")
	float CurrentAttackRange;

	//身體半徑，讓巨大的單位不需要很長的攻擊距離才能攻擊到
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon")
	float BodySize = 100;

	//攻擊計時器
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current", Replicated)
	float AttackingCounting = 0;

	/*

	                             Each Attacking Time gap
	|---------------------------------------------------------------------------------------|
	                                                                 waiting for next attack
	                                                                |-----------------------|
	CurrentAttackingAnimationTimeLength
	|---------------------------------------------------------------|
	CurrentAttackingBeginingTimeLength
	|--------------------------------|
	  								 CurrentAttackingEndingTimeLength
									 |------------------------------|
									 ^
							    Cause Damage
	*/
	//攻擊動畫時間長度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current")
	float CurrentAttackingAnimationTimeLength = 0.5;
	//攻擊動畫播放速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current", Replicated)
	float CurrentAttackingAnimationRate = 1;
	//攻擊前搖時間長度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current", Replicated)
	float CurrentAttackingBeginingTimeLength = 0.3;
	//攻擊後搖時間長度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current", Replicated)
	float CurrentAttackingEndingTimeLength = 0.2;
	//當前普攻是否打出來了
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current")
	bool IsAttacked = false;

	//攻速
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current")
	float CurrentAttackSpeed = 0;
	//攻速秒數
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current", Replicated)
	float CurrentAttackSpeedSecond;
	//攻擊力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current")
	float CurrentAttack;
	//基礎攻速
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon")
	float BaseAttackSpeedSecond = 1.8;


	//追踨目標計時器
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Counting")
	float FollowActorUpdateCounting = 0;
	//追踨目標更新時間
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon")
	float FollowActorUpdateTimeGap = 0.3;

	//依序做完裡面的動作
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current")
	TArray<FHeroAction> ActionQueue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "aon|Current")
	FHeroAction CurrentAction;
	
	UPROPERTY(EditAnywhere, meta = (DisplayName = "攻擊順序"), Category = "aon")
	EShowMethod AttackShowMethod;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "攻擊動作"), Category = "aon")
	TArray<UAnimMontage*> AttackMontages;

	// 給攻擊順序使用
	UPROPERTY(EditAnywhere, meta = (DisplayName = "攻擊index"), Category = "aon")
	int32 AttackIndex = 0;

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

	FOnHPChangeDelegate OnHPChangeDelegate;
	FOnMPChangeDelegate OnMPChangeDelegate;

protected:
	UFUNCTION()
	void OnRep_HP();

	UFUNCTION()
	void OnRep_MP();

	UFUNCTION()
	void OnRep_MaxHP();
	
	UFUNCTION()
	void OnRep_MaxMP();

private:
	void DoAction_AttackActor(const FHeroAction& CurrentAction);
};
