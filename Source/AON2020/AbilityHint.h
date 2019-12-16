// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperSpriteComponent.h"
#include "Components/SceneComponent.h"
#include "AbilityHint.generated.h"

UENUM(BlueprintType)
enum class EAbilityHint : uint8
{
	NoneHint,
	Direction,
	Range,
	EarmarkHero,
	EarmarkNonHero,
	EarmarkAnyone,
};

UCLASS()
class AON2020_API AAbilityHint : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAbilityHint();

	UPROPERTY(Category = "aon", VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Scene;

	UPROPERTY(Category = "aon", VisibleAnywhere, BlueprintReadOnly)
	UPaperSpriteComponent* BodySprite;

	UPROPERTY(Category = "aon", VisibleAnywhere, BlueprintReadOnly)
	UPaperSpriteComponent* HeadSprite;

	UPROPERTY(Category = "aon", VisibleAnywhere, BlueprintReadOnly)
	UPaperSpriteComponent* FootSprite;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostInitProperties() override;
#endif
	UFUNCTION(Category = "aon", BlueprintCallable)
	void SetLength(float len);

	void UpdateLength();

	void UpdatePos(FVector PlayerPos, FVector MousePos);
	
	UPROPERTY(Category = "SkillHint", EditAnywhere, BlueprintReadOnly)
	EAbilityHint SkillType = EAbilityHint::NoneHint;

	UPROPERTY(Category = "SkillHint", EditAnywhere, BlueprintReadOnly)
	FVector SkillPos;

	UPROPERTY(Category = "SkillHint", VisibleAnywhere, BlueprintReadOnly)
	bool UseDirectionSkill = false;

	UPROPERTY(Category = "SkillHint", VisibleAnywhere, BlueprintReadOnly)
	bool UseRangeSkill = false;
	
	// 是否固定長度
	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "UseDirectionSkill"))
	bool IsFixdLength= true;
	
	UPROPERTY(Category = "SkillHint", EditAnywhere, BlueprintReadOnly)
	UTexture2D* MouseIcon = nullptr;
	
	//範圍技直徑
	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "UseRangeSkill"))
	float SkillDiameter = 200;

	//最大施法距離
	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "UseRangeSkill"))
	float MaxCastRange = 500;

	//最小施法距離
	UPROPERTY(Category = "FlySkill", EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "UseRangeSkill"))
	float MinCastRange = 100;
};
