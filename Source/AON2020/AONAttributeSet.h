// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AONAttributeSet.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChangeDelegate, float, HP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMPChangeDelegate, float, MP, float, MaxMP);

/**
 * 
 */
UCLASS()
class AON2020_API UAONAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	class AUnit* Owner;

	//隊伍id
	UPROPERTY(EditAnywhere, Category = "aon")
	int32 TeamId = 0;
	
	UPROPERTY(EditAnywhere, Category = "aon", ReplicatedUsing = OnRep_HP)
	int16 HP = 400;
	UPROPERTY(EditAnywhere, Category = "aon", ReplicatedUsing = OnRep_MaxHP)
	int16 MaxHP = 400;

	UPROPERTY(EditAnywhere, Category = "aon", ReplicatedUsing = OnRep_MP)
	int16 MP = 1000;
	UPROPERTY(EditAnywhere, Category = "aon", ReplicatedUsing = OnRep_MaxMP)
	int16 MaxMP = 1000;

	//攻擊距離
	UPROPERTY()
	int16 AttackRange = 130;

	//通用護盾值
	UPROPERTY()
	int16 Shield = 0;
	//物理護盾值
	UPROPERTY()
	int16 ShieldPhysical = 0;
	//魔法護盾值
	UPROPERTY()
	int16 ShieldMagical = 0;
	//攻擊失誤率
	UPROPERTY()
	int16 AttackMiss = 0;
	
	// BuffProperty ===========================================
	//閃避率
	UPROPERTY()
	int16 Dodge = 0;
	//每秒回血
	UPROPERTY()
	float RegenHP;
	//每秒回魔
	UPROPERTY()
	float RegenMP;

	//攻速
	UPROPERTY()
	int16 AttackSpeed = 100;
	//魔法受傷倍率
	UPROPERTY()
	int16 MagicInjuredRatio = 100;
	//物理受傷倍率
	UPROPERTY()
	int16 PhysicsInjuredRatio = 100;
	//裝甲
	UPROPERTY()
	int16 Armor = 10;
	//攻擊力
	UPROPERTY()
	int16 Attack = 100;
	//移動速度
	UPROPERTY()
	int16 MoveSpeed = 300;
	//力量
	UPROPERTY()
	int16 Strength = 30;
	//敏捷
	UPROPERTY()
	int16 Agility = 30;
	//智力
	UPROPERTY()
	int16 Intelligence = 30;

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
};
