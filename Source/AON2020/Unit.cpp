// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit.h"
#include "AON2020.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

// Sets default values
AUnit::AUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

void AUnit::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUnit, BodyStatus);
}
