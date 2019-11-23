// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroController.h"
#include "AON2020.h"
#include <Blueprint/AIBlueprintHelperLibrary.h>
#include <WidgetLayoutLibrary.h>
#include "Kismet/GameplayStatics.h"

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
		if (FVector2D::DistSquared(CurrentMouseXY, FVector2D::ZeroVector) > 0.1)
		{
			TArray<FHitResult> Hits;
			bool res;
			FVector WorldOrigin;
			FVector WorldDirection;
			FCollisionObjectQueryParams CollisionQuery;
			CollisionQuery.AddObjectTypesToQuery(ECC_WorldStatic);
			if (UGameplayStatics::DeprojectScreenToWorld(this, CurrentMouseXY, WorldOrigin, WorldDirection) == true)
			{
				res = GetWorld()->LineTraceMultiByObjectType(Hits, WorldOrigin, WorldOrigin + WorldDirection * HitResultTraceDistance,
					CollisionQuery);
			}

			// 只trace 地板的actor
			// 地板名可以自定義
			FVector HitPoint = FVector::ZeroVector;
			if (Hits.Num() > 0)
			{
				for (FHitResult Hit : Hits)
				{
					if (Hit.Actor.IsValid() && Hit.Actor->GetFName().GetPlainNameString() == "Floor")
					{
						HitPoint = Hit.ImpactPoint;
					}
					// all landscape can click
					if (Hit.Actor.IsValid())
					{
						FString ResStr;
						Hit.Actor->GetClass()->GetName(ResStr);
						if (ResStr == "Landscape")
						{
							HitPoint = Hit.ImpactPoint;
						}
					}
				}
			}
			if (HitPoint != FVector::ZeroVector)
			{
				FVector pos = this->GetPawn()->GetActorLocation();
				HitPoint.Z = pos.Z;
				float dis = FVector::DistSquared(pos, HitPoint);
				
				if (dis < 4000)
				{
					HitPoint = HitPoint + (HitPoint - pos) * 15;
					float dis2 = FVector::DistSquared(pos, HitPoint);
					//UE_LOG(LogAON2020, Warning, TEXT("dis %f %f HitPoint %.f %.f %.f"), dis, dis2, HitPoint.X, HitPoint.Y, HitPoint.Z);
					UAIBlueprintHelperLibrary::SimpleMoveToLocation(LocalPlayer->GetPlayerController(GetWorld()), HitPoint);
				}
				else
				{
					UAIBlueprintHelperLibrary::SimpleMoveToLocation(LocalPlayer->GetPlayerController(GetWorld()), HitPoint);
				}
				
			}
		}
	}
}

void AHeroController::SetupInputComponent()
{
	Super::SetupInputComponent();

}
