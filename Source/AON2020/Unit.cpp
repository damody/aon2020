// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit.h"
#include "AON2020.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "HeroController.h"
#include "WebInterfaceJSON.h"
#include "WebInterfaceObject.h"
#include "WebInterfaceHelpers.h"

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

void AUnit::OverideAction2(FHeroAction action)
{
	action.SequenceNumber = Seq++;
	ActionQueue.Empty();
	ActionQueue.Add(action);
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
		}
		else
		{
			FollowActorUpdateCounting = 0;
			FVector pos = GetActorLocation();
			FVector v = (TargetActor->GetActorLocation() - pos);
			v.Z = 0;
			v.Normalize();
			this->SetActorRotation(v.Rotation());
			this->AddMovementInput(v);
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
				if (AttackShowMethod == EShowMethod::SEQUENCE)
				{
					if (AttackIndex >= AttackMontages.Num())
					{
						AttackIndex = 0;
					}
					Montage_Play(AttackMontages[AttackIndex]);
					AttackIndex++;
				}
				else
				{
					AttackIndex = FMath::Rand() % AttackMontages.Num();
					Montage_Play(AttackMontages[AttackIndex]);
				}
			}
		}
	}
	break;
	case EHeroBodyStatus::AttackBegining:
	{
		if (!IsAttacked && AttackingCounting > CurrentAttackingBeginingTimeLength)
		{
			AHeroController* controller = Cast<AHeroController>(GetController());
			if (GetWorld()->GetNetMode() == NM_DedicatedServer)
			{
				// 遠攻傷害
// 				if (AttackBullet)
// 				{
// 					FVector pos = GetActorLocation();
// 					ABulletActor* bullet = GetWorld()->SpawnActor<ABulletActor>(AttackBullet);
// 					if (bullet)
// 					{
// 						bullet->SetActorLocation(pos);
// 						bullet->SetTargetActor(this, TargetActor);
// 						bullet->Damage = this->CurrentAttack;
// 					}
// 				}
// 				else
				{// 近戰傷害
					if (controller)
					{
						controller->ServerAttackCompute(this, TargetActor, EDamageType::DAMAGE_PHYSICAL, CurrentAttack, true);
					}
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
		this->StopAnimMontage();
		BodyStatus = EHeroBodyStatus::Moving;
	}
	break;
	case EHeroBodyStatus::Moving:
	{
		FVector HitPoint = CurrentAction.TargetVec1;
		FVector pos = GetActorLocation();
		float dis = FVector::DistSquared(pos, HitPoint);
		if (dis < 8000)
		{
			FVector v = (HitPoint - pos);
			v.Z = 0;
			v.Normalize();
			HitPoint = HitPoint + v * 40;
		}

		FVector v = (HitPoint - pos);
		v.Z = 0;
		v.Normalize();
		this->SetActorRotation(v.Rotation());
		this->AddMovementInput(v);
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


UWebInterfaceJsonObject* AUnit::BuildJsonObject()
{
	UWebInterfaceJsonObject* wjo = UWebInterfaceHelpers::ConstructObject();
	//一般單位也有的屬性
	//英雄名/單位名
	wjo->SetString(FString(TEXT("UnitName")), UnitName);
	//隊伍id
	wjo->SetInteger(FString(TEXT("TeamId")), TeamId);
	//是否活著
	wjo->SetBoolean(FString(TEXT("IsAlive")), IsAlive);
	//移動速度
	wjo->SetInteger(FString(TEXT("CurrentMoveSpeed")), 0);
	//最大血量
	wjo->SetInteger(FString(TEXT("CurrentMaxHP")), MaxHP);
	//血量
	wjo->SetInteger(FString(TEXT("CurrentHP")), HP);
	//通用護盾值
	wjo->SetInteger(FString(TEXT("CurrentShield")), 0);
	//物理護盾值
	wjo->SetInteger(FString(TEXT("CurrentShieldPhysical")), 0);
	//魔法護盾值
	wjo->SetInteger(FString(TEXT("CurrentShieldMagical")), 0);
	//最大魔力
	wjo->SetInteger(FString(TEXT("CurrentMaxMP")), MaxMP);
	//魔力
	wjo->SetInteger(FString(TEXT("CurrentMP")), MP);
	//每秒回血
	wjo->SetNumber(FString(TEXT("CurrentRegenHP")), 0);
	//每秒回魔
	wjo->SetNumber(FString(TEXT("CurrentRegenMP")), 0);
	//攻速
	wjo->SetNumber(FString(TEXT("CurrentAttackSpeed")), CurrentAttackSpeed);
	//攻速秒數
	wjo->SetNumber(FString(TEXT("CurrentAttackSpeedSecond")), CurrentAttackSpeedSecond);
	//攻擊力
	wjo->SetInteger(FString(TEXT("CurrentAttack")), CurrentAttack);
	//防禦力
	wjo->SetNumber(FString(TEXT("CurrentArmor")), 0);
	//攻擊距離
	wjo->SetInteger(FString(TEXT("CurrentAttackRange")), CurrentAttackRange);
	//當前魔法受傷倍率
	wjo->SetNumber(FString(TEXT("CurrentMagicInjured")), 0);
	//準備要用的技能index
	wjo->SetInteger(FString(TEXT("CurrentSkillIndex")), 0);
	//剩餘的升級技能點數
	wjo->SetInteger(FString(TEXT("CurrentSkillPoints")), 0);
	//暈炫倒數計時器
	wjo->SetInteger(FString(TEXT("StunningLeftCounting")), 0);
	//死亡給敵金錢
	wjo->SetInteger(FString(TEXT("BountyGold")), 0);
	//基礎攻擊力
	wjo->SetInteger(FString(TEXT("BaseAttack")), 0);
	//基礎裝甲
	wjo->SetNumber(FString(TEXT("BaseArmor")), 0);
	//基礎移動速度
	wjo->SetInteger(FString(TEXT("BaseMoveSpeed")), 0);
	//基礎攻擊距離
	wjo->SetInteger(FString(TEXT("BaseAttackRange")), 0);
	//技能數量
	wjo->SetNumber(FString::Printf(TEXT("Skill_Amount")), 0);
	//Buff數量
	wjo->SetNumber(FString::Printf(TEXT("Buff_Amount")), 0);
// 	for (int i = 0; i < this->Skills.Num(); ++i)
// 	{
// 		if (IsValid(this->Skills[i]))
// 		{
// 			//技能名稱
// 			wjo->SetString(FString::Printf(TEXT("Skill%d_Name"), i + 1), this->Skills[i]->Name);
// 			//是否啟用
// 			wjo->SetBoolean(FString::Printf(TEXT("Skill%d_Enabled"), i + 1), this->Skills[i]->IsEnable());
// 			//是否開關
// 			wjo->SetBoolean(FString::Printf(TEXT("Skill%d_Toggle"), i + 1), this->Skills[i]->Toggle);
// 			//是否顯示
// 			wjo->SetBoolean(FString::Printf(TEXT("Skill%d_Display"), i + 1), this->Skills[i]->IsDisplay());
// 			//圖片路徑
// 			wjo->SetString(FString::Printf(TEXT("Skill%d_Webpath"), i + 1), this->Skills[i]->Webpath);
// 			//技能描述
// 			wjo->SetString(FString::Printf(TEXT("Skill%d_Description"), i + 1), this->Skills[i]->GetDescription());
// 			//CD百分比
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_CDPercent"), i + 1), this->Skills[i]->GetSkillCDPercent());
// 			//目前CD時間
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_CurrentCD"), i + 1), this->Skills[i]->CurrentCD);
// 			//目前最大CD時間
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_MaxCD"), i + 1), this->Skills[i]->MaxCD);
// 			//該技能目前可不可以升級
// 			if (this->Skills[i]->CanLevelUp() && CurrentSkillPoints > 0)
// 			{
// 				wjo->SetBoolean(FString::Printf(TEXT("Skill%d_CanLevelUp"), i + 1), true);
// 			}
// 			else
// 			{
// 				wjo->SetBoolean(FString::Printf(TEXT("Skill%d_CanLevelUp"), i + 1), false);
// 			}
// 			//技能等級
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_CurrentLevel"), i + 1), this->Skills[i]->CurrentLevel);
// 			//技能最大等級
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_MaxLevel"), i + 1), this->Skills[i]->MaxLevel);
// 		}
// 	}
// 	for (int i = 0; i < Buffs.Num(); ++i)
// 	{
// 		if (IsValid(Buffs[i]))
// 		{
// 			//Buff名稱
// 			wjo->SetString(FString::Printf(TEXT("Buff%d_Name"), i + 1), Buffs[i]->Name);
// 			//圖片路徑
// 			wjo->SetString(FString::Printf(TEXT("Buff%d_Webpath"), i + 1), Buffs[i]->Webpath);
// 			//是否是正面Buff
// 			wjo->SetBoolean(FString::Printf(TEXT("Buff%d_Friendly"), i + 1), Buffs[i]->Friendly);
// 			//Buff提示
// 			wjo->SetString(FString::Printf(TEXT("Buff%d_BuffTips"), i + 1), Buffs[i]->BuffTips);
// 			//Buff堆疊成數
// 			wjo->SetNumber(FString::Printf(TEXT("Buff%d_Stacks"), i + 1), Buffs[i]->Stacks);
// 			//Buff持續剩餘時間
// 			wjo->SetNumber(FString::Printf(TEXT("Buff%d_Duration"), i + 1), Buffs[i]->Duration);
// 			//Buff持續總時間
// 			wjo->SetNumber(FString::Printf(TEXT("Buff%d_MaxDuration"), i + 1), Buffs[i]->MaxDuration);
// 			//Buff是否可堆疊
// 			wjo->SetBoolean(FString::Printf(TEXT("Buff%d_CanStacks"), i + 1), Buffs[i]->CanStacks);
// 		}
// 	}
	return wjo;
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
	//DOREPLIFETIME(AUnit, ActionQueue);
}
