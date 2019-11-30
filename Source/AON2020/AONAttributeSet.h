// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AONAttributeSet.generated.h"


/**
 * 
 */
UCLASS()
class AON2020_API UAONAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
