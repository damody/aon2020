// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroController.h"
#include "AON2020.h"
#include <Blueprint/AIBlueprintHelperLibrary.h>
#include <WidgetLayoutLibrary.h>
#include "Kismet/GameplayStatics.h"
#include "IXRTrackingSystem.h"
#include "Engine.h"

AHeroController::AHeroController()
{
	bShowMouseCursor = true;
}

FVector2D AHeroController::GetMouseScreenPosition()
{
	// change to UWidgetLayoutLibrary::GetMousePositionOnViewport 拿到viewport的座標
// 	FVector2D mouseposofview = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
// 	float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
// 	mouseposofview.X *= ViewportScale;
// 	mouseposofview.Y *= ViewportScale;
// 	return mouseposofview;
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D res;
		LocalPlayer->ViewportClient->GetMousePosition(res);
		CurrentMouseXY = res;
	}
	else
	{
		CurrentMouseXY = FVector2D::ZeroVector;
	}
	return CurrentMouseXY;
}

void AHeroController::OnMouseLDown()
{

}

void AHeroController::OnMouseRDown()
{
	if (FVector2D::DistSquared(CurrentMouseXY, FVector2D::ZeroVector) > 0.1)
	{
		TArray<FHitResult> Hits;
		bool res;
		FVector WorldOrigin;
		FVector WorldDirection;
		// 點人專用
		{
			FCollisionObjectQueryParams CollisionQuery;
			CollisionQuery.AddObjectTypesToQuery(ECC_Pawn);
			if (UGameplayStatics::DeprojectScreenToWorld(this, CurrentMouseXY, WorldOrigin, WorldDirection) == true)
			{
				res = GetWorld()->LineTraceMultiByObjectType(Hits, WorldOrigin, WorldOrigin + WorldDirection * HitResultTraceDistance,
					CollisionQuery);
			}
			FVector HitPoint = FVector::ZeroVector;
			AUnit* TargetUnit = nullptr;
			if (Hits.Num() > 0)
			{
				for (FHitResult Hit : Hits)
				{
					// 有點到東西
					if (Hit.Actor.IsValid())
					{
						TargetUnit = Cast<AUnit>(Hit.Actor);
						if (TargetUnit)
						{
							GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Click Hero %s"), *TargetUnit->GetClass()->GetName()));
						}
						else
						{
							UE_LOG(LogAON2020, Warning, TEXT("Click Actor %s"), *Hit.Actor->GetClass()->GetName());
						}
					}

				}
			}
			ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
			if (LocalPlayer && TargetUnit)
			{
				AUnit* unit = Cast<AUnit>(this->GetPawn());
				if (unit && unit != TargetUnit)
				{
					FHeroAction action;
					action.ActionStatus = EHeroActionStatus::AttackActor;
					action.TargetActor = TargetUnit;
					unit->OverideAction(action);
					unit->OverideAction2(action);
					return;
				}
			}
		}
		// 點地版專用
		{
			FCollisionObjectQueryParams CollisionQuery;
			CollisionQuery.AddObjectTypesToQuery(ECC_WorldStatic);
			if (UGameplayStatics::DeprojectScreenToWorld(this, CurrentMouseXY, WorldOrigin, WorldDirection) == true)
			{
				res = GetWorld()->LineTraceMultiByObjectType(Hits, WorldOrigin, WorldOrigin + WorldDirection * HitResultTraceDistance,
					CollisionQuery);
			}
			// 只 trace WorldStatic 的地板
			FVector HitPoint = FVector::ZeroVector;
			if (Hits.Num() > 0)
			{
				for (FHitResult Hit : Hits)
				{
					// 有點到東西
					if (Hit.Actor.IsValid())
					{
						// 點到地板
						if (Hit.Actor->GetFName().GetPlainNameString() == "Floor")
						{
							HitPoint = Hit.ImpactPoint;
						}
						// 點到地板
						if (Hit.Actor.IsValid())
						{
							FString ResStr;
							Hit.Actor->GetClass()->GetName(ResStr);
							if (ResStr == "Landscape")
							{
								HitPoint = Hit.ImpactPoint;
							}
						}
						AUnit* unit = Cast<AUnit>(Hit.Actor);
						if (unit)
						{
							UE_LOG(LogAON2020, Warning, TEXT("Click Actor %s"), *unit->GetClass()->GetName());
						}
						else
						{
							UE_LOG(LogAON2020, Warning, TEXT("Click Actor %s"), *Hit.Actor->GetClass()->GetName());
						}
					}

				}
			}
			ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
			if (LocalPlayer && HitPoint != FVector::ZeroVector)
			{
				AUnit* unit = Cast<AUnit>(this->GetPawn());
				if (unit)
				{
					FVector pos = unit->GetActorLocation();
					HitPoint.Z = pos.Z;
					FHeroAction action;
					action.ActionStatus = EHeroActionStatus::MoveToPosition;
					action.TargetVec1 = HitPoint;
					unit->OverideAction(action);
					unit->OverideAction2(action);
				}
			}
		}

		
		
	}
}

void AHeroController::ServerAttackCompute_Implementation(AUnit* attacker, AUnit* victim, EDamageType dtype, float damage, bool AttackLanded)
{
	victim->HP -= damage;
}

bool AHeroController::ServerAttackCompute_Validate(AUnit* attacker, AUnit* victim, EDamageType dtype, float damage, bool AttackLanded)
{
	return true;
}

void AHeroController::ServerCharacterMove_Implementation(class AUnit* hero, const FVector& pos)
{
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(hero->GetController(), pos);
}

bool AHeroController::ServerCharacterMove_Validate(class AUnit* hero, const FVector& pos)
{
	return true;
}

void AHeroController::BeginPlay()
{
	Super::BeginPlay();
}

void AHeroController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D MousePosition;
		FHitResult HitResult;
		bool bHit = false;

		bHit = GetHitResultAtScreenPosition(GetMouseScreenPosition(), CurrentClickTraceChannel, true, /*out*/ HitResult);

		UPrimitiveComponent* PreviousComponent = CurrentClickablePrimitive.Get();
		UPrimitiveComponent* CurrentComponent = (bHit ? HitResult.Component.Get() : NULL);

		UPrimitiveComponent::DispatchMouseOverEvents(PreviousComponent, CurrentComponent);
		if (IsValid(CurrentComponent))
		{
			CurrentClickablePrimitive = CurrentComponent;
		}
		else
		{
			CurrentClickablePrimitive = 0;
		}
	}
}

void AHeroController::SetupInputComponent()
{
	Super::SetupInputComponent();

}

bool AHeroController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	if (GEngine->XRSystem.IsValid())
	{
		auto XRInput = GEngine->XRSystem->GetXRInput();
		if (XRInput && XRInput->HandleInputKey(PlayerInput, Key, EventType, AmountDepressed, bGamepad))
		{
			return true;
		}
	}
	if (Key == EKeys::LeftShift)
	{
		if (EventType == IE_Pressed)
		{
			bLeftShiftDown = true;
		}
		else if (EventType == IE_Released)
		{
			bLeftShiftDown = false;
		}
	}
	else if (Key == EKeys::RightShift)
	{
		if (EventType == IE_Pressed)
		{
			bRightShiftDown = true;
		}
		else if (EventType == IE_Released)
		{
			bRightShiftDown = false;
		}
	}
	else if (Key == EKeys::LeftMouseButton)
	{
		if (EventType == IE_Pressed)
		{
			bMouseLButton = true;
			OnMouseLDown();
		}
		else if (EventType == IE_Released)
		{
			bMouseLButton = false;
		}
	}
	else if (Key == EKeys::RightMouseButton)
	{
		if (EventType == IE_Pressed)
		{
			bMouseRButton = true;
			OnMouseRDown();
		}
		else if (EventType == IE_Released)
		{
			bMouseRButton = false;
		}
	}

	bool bResult = false;
	if (PlayerInput)
	{
		bResult = PlayerInput->InputKey(Key, EventType, AmountDepressed, bGamepad);
		if (bEnableClickEvents && (ClickEventKeys.Contains(Key) || ClickEventKeys.Contains(EKeys::AnyKey)))
		{
			FVector2D MousePosition;
			UGameViewportClient* ViewportClient = CastChecked<ULocalPlayer>(Player)->ViewportClient;
			if (ViewportClient && ViewportClient->GetMousePosition(MousePosition))
			{
				UPrimitiveComponent* ClickedPrimitive = NULL;
				if (bEnableMouseOverEvents)
				{
					ClickedPrimitive = CurrentClickablePrimitive.Get();
				}
				else
				{
					FHitResult HitResult;
					const bool bHit = GetHitResultAtScreenPosition(MousePosition, CurrentClickTraceChannel, true, HitResult);
					if (bHit)
					{
						ClickedPrimitive = HitResult.Component.Get();
					}
				}
				if (GetHUD())
				{
					if (GetHUD()->UpdateAndDispatchHitBoxClickEvents(MousePosition, EventType))
					{
						ClickedPrimitive = NULL;
					}
				}

				if (ClickedPrimitive)
				{
					switch (EventType)
					{
					case IE_Pressed:
					case IE_DoubleClick:
						ClickedPrimitive->DispatchOnClicked(Key);
						break;

					case IE_Released:
						ClickedPrimitive->DispatchOnReleased(Key);
						break;

					case IE_Axis:
					case IE_Repeat:
						break;
					}
				}

				bResult = true;
			}
		}
	}

	return bResult;
}
