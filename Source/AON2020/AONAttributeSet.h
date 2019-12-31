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

	//����id
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

	//�����Z��
	UPROPERTY()
	int16 AttackRange = 130;

	//�q���@�ޭ�
	UPROPERTY()
	int16 Shield = 0;
	//���z�@�ޭ�
	UPROPERTY()
	int16 ShieldPhysical = 0;
	//�]�k�@�ޭ�
	UPROPERTY()
	int16 ShieldMagical = 0;
	//�������~�v
	UPROPERTY()
	int16 AttackMiss = 0;
	
	// BuffProperty ===========================================
	//�{�ײv
	UPROPERTY()
	int16 Dodge = 0;
	//�C��^��
	UPROPERTY()
	float RegenHP;
	//�C��^�]
	UPROPERTY()
	float RegenMP;

	//��t
	UPROPERTY()
	int16 AttackSpeed = 100;
	//�]�k���˭��v
	UPROPERTY()
	int16 MagicInjuredRatio = 100;
	//���z���˭��v
	UPROPERTY()
	int16 PhysicsInjuredRatio = 100;
	//�˥�
	UPROPERTY()
	int16 Armor = 10;
	//�����O
	UPROPERTY()
	int16 Attack = 100;
	//���ʳt��
	UPROPERTY()
	int16 MoveSpeed = 300;
	//�O�q
	UPROPERTY()
	int16 Strength = 30;
	//�ӱ�
	UPROPERTY()
	int16 Agility = 30;
	//���O
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
