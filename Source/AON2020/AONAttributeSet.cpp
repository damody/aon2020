// Fill out your copyright notice in the Description page of Project Settings.


#include "AONAttributeSet.h"

void UAONAttributeSet::OnRep_HP()
{
	HP = FMath::Clamp(HP, (int16)0, MaxHP);
	OnHPChangeDelegate.Broadcast(HP, MaxHP);
}

void UAONAttributeSet::OnRep_MaxHP()
{
	HP = FMath::Clamp(HP, (int16)0, MaxHP);
	OnHPChangeDelegate.Broadcast(HP, MaxHP);
}

void UAONAttributeSet::OnRep_MP()
{
	MP = FMath::Clamp(MP, (int16)0, MaxMP);
	OnMPChangeDelegate.Broadcast(MP, MaxMP);
}

void UAONAttributeSet::OnRep_MaxMP()
{
	MP = FMath::Clamp(MP, (int16)0, MaxMP);
	OnMPChangeDelegate.Broadcast(MP, MaxMP);
}


void UAONAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}
