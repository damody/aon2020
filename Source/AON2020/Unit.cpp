// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit.h"
#include "AON2020.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "HeroController.h"


// Sets default values
AUnit::AUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystem = CreateDefaultSubobject<UAONAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystem->SetIsReplicated(true);
	AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();
	OnHPChange(HP, MaxHP);
	OnMPChange(MP, MaxMP);
}

// Called every frame
void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ActionLoop();
	AttackingCounting += DeltaTime;
	FollowActorUpdateCounting += DeltaTime;
	CurrentAttackSpeedSecond = BaseAttackSpeedSecond / (1 + CurrentAttackSpeed);
// 	FVector dir = GetVelocity();
// 	dir.Z = 0;
// 	this->SetActorRotation(dir.Rotation());
}

void AUnit::ActionLoop()
{
	// 是否有動作？
	if (ActionQueue.Num() > 0 && IsAlive && EHeroBodyStatus::Stunning != BodyStatus)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Magenta, FString::Printf(L"ActionQueue %d", ActionQueue.Num()));
		// 動作駐列最上層動作是否為當前動作
		if (ActionQueue[0] != CurrentAction)
		{
			// 拿出動作
			CurrentAction = ActionQueue[0];
			// 進入此狀態的有限狀態機來做事
			DoAction(CurrentAction);
		}
		// 查看當前動作是否做完？
		else if (!CheckCurrentActionFinish())
		{
			// 進入此狀態的有限狀態機來做事
			DoAction(CurrentAction);
		}
		else
		{
			// 推出事件
			PopAction();
			// 檢查動作駐列是否為空？
			if (ActionQueue.Num() == 0)
			{
				// 站立不動
				DoNothing();
			}
			else
			{
				// 進入此狀態的有限狀態機來做事
				DoAction(CurrentAction);
			}
		}
	}
	else
	{
		// 站立不動
		DoNothing();
	}
}

// Called to bind functionality to input
void AUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AUnit::Montage_Play_Implementation(UAnimMontage* MontageToPlay, float InPlayRate /*= 1.f*/)
{
	GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay);
}

bool AUnit::Montage_Play_Validate(UAnimMontage* MontageToPlay, float InPlayRate /*= 1.f*/)
{
	return true;
}

void AUnit::DoAction(const FHeroAction& CurrentAction)
{
	switch (CurrentAction.ActionStatus)
	{
	case EHeroActionStatus::Default:
		PopAction();
		break;
	case EHeroActionStatus::MoveToPosition:
		DoAction_MoveToPosition(CurrentAction);
		break;
	case EHeroActionStatus::AttackActor:
		DoAction_AttackActor(CurrentAction);
		break;
	default:
		break;
	}
}

void AUnit::DoNothing()
{
	switch (BodyStatus)
	{
	case EHeroBodyStatus::Standing:
		break;
	case EHeroBodyStatus::Moving:
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this->GetController(), GetActorLocation());
		BodyStatus = EHeroBodyStatus::Standing;
	}
	break;
	case EHeroBodyStatus::Stunning:
		break;
	default:
		BodyStatus = EHeroBodyStatus::Standing;
		break;
	}
}


bool AUnit::CheckCurrentActionFinish()
{
	switch (BodyStatus)
	{
	case EHeroBodyStatus::Standing:
	break;
	case EHeroBodyStatus::Moving:
	{
		// 移動到夠接近就 Pop 掉
		if (CurrentAction.ActionStatus == EHeroActionStatus::MoveToPosition)
		{
			float Distance = FVector::Dist(CurrentAction.TargetVec1, this->GetActorLocation());
			if (Distance < 10)
			{
				return true;
			}
		}
	}
	break;
	}
	return false;
}

void AUnit::PopAction()
{
	if (ActionQueue.Num() > 0)
	{
		ActionQueue.RemoveAt(0);
		LastMoveTarget = FVector::ZeroVector;
		if (ActionQueue.Num() > 0)
		{
			CurrentAction = ActionQueue[0];
		}
		else
		{
			CurrentAction.ActionStatus = EHeroActionStatus::Default;
		}
	}
}

void AUnit::DoAction_AttackActor(const FHeroAction& CurrentAction)
{
	AUnit* TargetActor = CurrentAction.TargetActor;
	FVector dir = TargetActor->GetActorLocation() - GetActorLocation();
	dir.Z = 0;
	dir.Normalize();
	SetActorRotation(dir.Rotation());
	switch (BodyStatus)
	{
	case EHeroBodyStatus::Standing:
	{
		float DistanceToTargetActor = FVector::Dist(TargetActor->GetActorLocation(), this->GetActorLocation());
		if (CurrentAttackRange + TargetActor->BodySize > DistanceToTargetActor)
		{
			BodyStatus = EHeroBodyStatus::AttackWating;
			IsAttacked = false;
		}
		else
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this->GetController(), TargetActor->GetActorLocation());
			BodyStatus = EHeroBodyStatus::Moving;
		}
	}
	break;
	case EHeroBodyStatus::Moving:
	{
		float DistanceToTargetActor = FVector::Dist(TargetActor->GetActorLocation(), this->GetActorLocation());
		if (CurrentAttackRange + TargetActor->BodySize > DistanceToTargetActor)
		{
			BodyStatus = EHeroBodyStatus::AttackWating;
			IsAttacked = false;
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this->GetController(), this->GetActorLocation());
		}
		else if (FollowActorUpdateCounting > FollowActorUpdateTimeGap)
		{
			FollowActorUpdateCounting = 0;
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this->GetController(), TargetActor->GetActorLocation());
		}
	}
	break;
	case EHeroBodyStatus::Stunning:
		break;
	case EHeroBodyStatus::AttackWating:
	{
		if (AttackingCounting > CurrentAttackSpeedSecond)
		{
			AttackingCounting = 0;
			BodyStatus = EHeroBodyStatus::AttackBegining;
			// 播放攻擊動畫
			if (AttackMontages.Num() > 0)
			{
				Montage_Play(AttackMontages[0]);
			}
		}
	}
	break;
	case EHeroBodyStatus::AttackBegining:
	{
		if (!IsAttacked && AttackingCounting > CurrentAttackingBeginingTimeLength)
		{
			AHeroController* controller = Cast<AHeroController>(GetController());
			// 遠攻傷害
// 			if (AttackBullet)
// 			{
// 				FVector pos = GetActorLocation();
// 				ABulletActor* bullet = GetWorld()->SpawnActor<ABulletActor>(AttackBullet);
// 				if (bullet)
// 				{
// 					bullet->SetActorLocation(pos);
// 					bullet->SetTargetActor(this, TargetActor);
// 					bullet->Damage = this->CurrentAttack;
// 				}
// 			}
// 			else
			{// 近戰傷害
				if (controller)
				{
					controller->ServerAttackCompute(this, TargetActor, EDamageType::DAMAGE_PHYSICAL, CurrentAttack, true);
				}
			}
			BodyStatus = EHeroBodyStatus::AttackEnding;
		}
	}
	break;
	case EHeroBodyStatus::AttackEnding:
	{
		if (AttackingCounting > CurrentAttackingBeginingTimeLength + CurrentAttackingEndingTimeLength)
		{
			BodyStatus = EHeroBodyStatus::Standing;
		}
	}
	break;
	case EHeroBodyStatus::SpellWating:
	case EHeroBodyStatus::SpellBegining:
	case EHeroBodyStatus::SpellEnding:

	default:
		BodyStatus = EHeroBodyStatus::Standing;
		break;
	}

}

void AUnit::DoAction_MoveToPosition(const FHeroAction& CurrentAction)
{
	switch (BodyStatus)
	{
	case EHeroBodyStatus::AttackWating:
	case EHeroBodyStatus::AttackBegining:
	case EHeroBodyStatus::AttackEnding:
	case EHeroBodyStatus::Standing:
	{
		BodyStatus = EHeroBodyStatus::Moving;
	}
	break;
	case EHeroBodyStatus::Moving:
	{
		FVector HitPoint = CurrentAction.TargetVec1;
		FVector pos = GetActorLocation();
		float dis = FVector::DistSquared(pos, HitPoint);
		if ((dis <= 8000 && (LastMoveTarget == CurrentAction.TargetVec1 || LastMoveTarget == FVector::ZeroVector)) ||
			(dis > 8000 && LastMoveTarget != CurrentAction.TargetVec1))
		{
			LastMoveTarget = CurrentAction.TargetVec1;
			if (dis < 8000)
			{
				FVector v = (HitPoint - pos);
				v.Normalize();
				HitPoint = HitPoint + v * 40;
				LastMoveTarget = HitPoint;
				float dis2 = FVector::DistSquared(pos, HitPoint);
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this->GetController(), HitPoint);
			}
			else
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this->GetController(), HitPoint);
			}
		}
	}
	break;
	}
}

void AUnit::OverideAction_Implementation(FHeroAction action)
{
	action.SequenceNumber = Seq++;
	ActionQueue.Empty();
	ActionQueue.Add(action);
}

bool AUnit::OverideAction_Validate(FHeroAction action)
{
	return true;
}

void AUnit::PushAction_Implementation(FHeroAction action)
{
	action.SequenceNumber = Seq++;
	ActionQueue.Add(action);
}

bool AUnit::PushAction_Validate(FHeroAction action)
{
	return true;
}

void AUnit::OnHPChange(float HP, float MaxHP)
{
	BP_OnHPChange(HP, MaxHP);
}

void AUnit::OnMPChange(float MP, float MaxMP)
{
	BP_OnMPChange(MP, MaxMP);
}

void AUnit::OnRep_HP()
{
	HP = FMath::Clamp(HP, (int16)0, MaxHP);
	OnHPChangeDelegate.Broadcast(HP, MaxHP);
	OnHPChange(HP, MaxHP);
}

void AUnit::OnRep_MaxHP()
{
	HP = FMath::Clamp(HP, (int16)0, MaxHP);
	OnHPChangeDelegate.Broadcast(HP, MaxHP);
	OnHPChange(HP, MaxHP);
}

void AUnit::OnRep_MP()
{
	MP = FMath::Clamp(MP, (int16)0, MaxMP);
	OnMPChangeDelegate.Broadcast(MP, MaxMP);
	OnMPChange(MP, MaxMP);
}

void AUnit::OnRep_MaxMP()
{
	MP = FMath::Clamp(MP, (int16)0, MaxMP);
	OnMPChangeDelegate.Broadcast(MP, MaxMP);
	OnMPChange(MP, MaxMP);
}


void AUnit::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUnit, BodyStatus);
	DOREPLIFETIME(AUnit, HP);
	DOREPLIFETIME(AUnit, MaxHP);
	DOREPLIFETIME(AUnit, MP);
	DOREPLIFETIME(AUnit, MaxMP);
}
