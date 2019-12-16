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
	// �O�_���ʧ@�H
	if (ActionQueue.Num() > 0 && IsAlive && EHeroBodyStatus::Stunning != BodyStatus)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Magenta, FString::Printf(L"ActionQueue %d", ActionQueue.Num()));
		// �ʧ@�n�C�̤W�h�ʧ@�O�_����e�ʧ@
		if (ActionQueue[0] != CurrentAction)
		{
			// ���X�ʧ@
			CurrentAction = ActionQueue[0];
			// �i�J�����A���������A���Ӱ���
			DoAction(CurrentAction);
		}
		// �d�ݷ�e�ʧ@�O�_�����H
		else if (!CheckCurrentActionFinish())
		{
			// �i�J�����A���������A���Ӱ���
			DoAction(CurrentAction);
		}
		else
		{
			// ���X�ƥ�
			PopAction();
			// �ˬd�ʧ@�n�C�O�_���šH
			if (ActionQueue.Num() == 0)
			{
				// ���ߤ���
				DoNothing();
			}
			else
			{
				// �i�J�����A���������A���Ӱ���
				DoAction(CurrentAction);
			}
		}
	}
	else
	{
		// ���ߤ���
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
		// ���ʨ������N Pop ��
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
			// ��������ʵe
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
				// ����ˮ`
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
				{// ��Զˮ`
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
	//�@����]�����ݩ�
	//�^���W/���W
	wjo->SetString(FString(TEXT("UnitName")), UnitName);
	//����id
	wjo->SetInteger(FString(TEXT("TeamId")), TeamId);
	//�O�_����
	wjo->SetBoolean(FString(TEXT("IsAlive")), IsAlive);
	//���ʳt��
	wjo->SetInteger(FString(TEXT("CurrentMoveSpeed")), 0);
	//�̤j��q
	wjo->SetInteger(FString(TEXT("CurrentMaxHP")), MaxHP);
	//��q
	wjo->SetInteger(FString(TEXT("CurrentHP")), HP);
	//�q���@�ޭ�
	wjo->SetInteger(FString(TEXT("CurrentShield")), 0);
	//���z�@�ޭ�
	wjo->SetInteger(FString(TEXT("CurrentShieldPhysical")), 0);
	//�]�k�@�ޭ�
	wjo->SetInteger(FString(TEXT("CurrentShieldMagical")), 0);
	//�̤j�]�O
	wjo->SetInteger(FString(TEXT("CurrentMaxMP")), MaxMP);
	//�]�O
	wjo->SetInteger(FString(TEXT("CurrentMP")), MP);
	//�C��^��
	wjo->SetNumber(FString(TEXT("CurrentRegenHP")), 0);
	//�C��^�]
	wjo->SetNumber(FString(TEXT("CurrentRegenMP")), 0);
	//��t
	wjo->SetNumber(FString(TEXT("CurrentAttackSpeed")), CurrentAttackSpeed);
	//��t���
	wjo->SetNumber(FString(TEXT("CurrentAttackSpeedSecond")), CurrentAttackSpeedSecond);
	//�����O
	wjo->SetInteger(FString(TEXT("CurrentAttack")), CurrentAttack);
	//���m�O
	wjo->SetNumber(FString(TEXT("CurrentArmor")), 0);
	//�����Z��
	wjo->SetInteger(FString(TEXT("CurrentAttackRange")), CurrentAttackRange);
	//��e�]�k���˭��v
	wjo->SetNumber(FString(TEXT("CurrentMagicInjured")), 0);
	//�ǳƭn�Ϊ��ޯ�index
	wjo->SetInteger(FString(TEXT("CurrentSkillIndex")), 0);
	//�Ѿl���ɯŧޯ��I��
	wjo->SetInteger(FString(TEXT("CurrentSkillPoints")), 0);
	//�w���˼ƭp�ɾ�
	wjo->SetInteger(FString(TEXT("StunningLeftCounting")), 0);
	//���`���Ī���
	wjo->SetInteger(FString(TEXT("BountyGold")), 0);
	//��¦�����O
	wjo->SetInteger(FString(TEXT("BaseAttack")), 0);
	//��¦�˥�
	wjo->SetNumber(FString(TEXT("BaseArmor")), 0);
	//��¦���ʳt��
	wjo->SetInteger(FString(TEXT("BaseMoveSpeed")), 0);
	//��¦�����Z��
	wjo->SetInteger(FString(TEXT("BaseAttackRange")), 0);
	//�ޯ�ƶq
	wjo->SetNumber(FString::Printf(TEXT("Skill_Amount")), 0);
	//Buff�ƶq
	wjo->SetNumber(FString::Printf(TEXT("Buff_Amount")), 0);
// 	for (int i = 0; i < this->Skills.Num(); ++i)
// 	{
// 		if (IsValid(this->Skills[i]))
// 		{
// 			//�ޯ�W��
// 			wjo->SetString(FString::Printf(TEXT("Skill%d_Name"), i + 1), this->Skills[i]->Name);
// 			//�O�_�ҥ�
// 			wjo->SetBoolean(FString::Printf(TEXT("Skill%d_Enabled"), i + 1), this->Skills[i]->IsEnable());
// 			//�O�_�}��
// 			wjo->SetBoolean(FString::Printf(TEXT("Skill%d_Toggle"), i + 1), this->Skills[i]->Toggle);
// 			//�O�_���
// 			wjo->SetBoolean(FString::Printf(TEXT("Skill%d_Display"), i + 1), this->Skills[i]->IsDisplay());
// 			//�Ϥ����|
// 			wjo->SetString(FString::Printf(TEXT("Skill%d_Webpath"), i + 1), this->Skills[i]->Webpath);
// 			//�ޯ�y�z
// 			wjo->SetString(FString::Printf(TEXT("Skill%d_Description"), i + 1), this->Skills[i]->GetDescription());
// 			//CD�ʤ���
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_CDPercent"), i + 1), this->Skills[i]->GetSkillCDPercent());
// 			//�ثeCD�ɶ�
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_CurrentCD"), i + 1), this->Skills[i]->CurrentCD);
// 			//�ثe�̤jCD�ɶ�
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_MaxCD"), i + 1), this->Skills[i]->MaxCD);
// 			//�ӧޯ�ثe�i���i�H�ɯ�
// 			if (this->Skills[i]->CanLevelUp() && CurrentSkillPoints > 0)
// 			{
// 				wjo->SetBoolean(FString::Printf(TEXT("Skill%d_CanLevelUp"), i + 1), true);
// 			}
// 			else
// 			{
// 				wjo->SetBoolean(FString::Printf(TEXT("Skill%d_CanLevelUp"), i + 1), false);
// 			}
// 			//�ޯ൥��
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_CurrentLevel"), i + 1), this->Skills[i]->CurrentLevel);
// 			//�ޯ�̤j����
// 			wjo->SetNumber(FString::Printf(TEXT("Skill%d_MaxLevel"), i + 1), this->Skills[i]->MaxLevel);
// 		}
// 	}
// 	for (int i = 0; i < Buffs.Num(); ++i)
// 	{
// 		if (IsValid(Buffs[i]))
// 		{
// 			//Buff�W��
// 			wjo->SetString(FString::Printf(TEXT("Buff%d_Name"), i + 1), Buffs[i]->Name);
// 			//�Ϥ����|
// 			wjo->SetString(FString::Printf(TEXT("Buff%d_Webpath"), i + 1), Buffs[i]->Webpath);
// 			//�O�_�O����Buff
// 			wjo->SetBoolean(FString::Printf(TEXT("Buff%d_Friendly"), i + 1), Buffs[i]->Friendly);
// 			//Buff����
// 			wjo->SetString(FString::Printf(TEXT("Buff%d_BuffTips"), i + 1), Buffs[i]->BuffTips);
// 			//Buff���|����
// 			wjo->SetNumber(FString::Printf(TEXT("Buff%d_Stacks"), i + 1), Buffs[i]->Stacks);
// 			//Buff����Ѿl�ɶ�
// 			wjo->SetNumber(FString::Printf(TEXT("Buff%d_Duration"), i + 1), Buffs[i]->Duration);
// 			//Buff�����`�ɶ�
// 			wjo->SetNumber(FString::Printf(TEXT("Buff%d_MaxDuration"), i + 1), Buffs[i]->MaxDuration);
// 			//Buff�O�_�i���|
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
