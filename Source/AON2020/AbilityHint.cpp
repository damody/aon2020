// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityHint.h"
#include "PaperSprite.h"

// Sets default values
AAbilityHint::AAbilityHint(const FObjectInitializer& ObjectInitializer)
	: Super(FObjectInitializer::Get())
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;
	PrimaryActorTick.bCanEverTick = false;
	Scene = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("root0"));
	BodySprite = ObjectInitializer.CreateDefaultSubobject<UPaperSpriteComponent>(this, TEXT("VisualizeBodySprite0"));
	HeadSprite = ObjectInitializer.CreateDefaultSubobject<UPaperSpriteComponent>(this, TEXT("VisualizeHeadSprite0"));
	FootSprite = ObjectInitializer.CreateDefaultSubobject<UPaperSpriteComponent>(this, TEXT("VisualizeFootSprite0"));
	RootComponent = Scene;
	if (BodySprite)
	{
		BodySprite->SetupAttachment(Scene);
		BodySprite->AlwaysLoadOnClient = true;
		BodySprite->AlwaysLoadOnServer = false;
		BodySprite->bOwnerNoSee = false;
		BodySprite->bAffectDynamicIndirectLighting = true;
		BodySprite->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		//BodySprite->bGenerateOverlapEvents = false;
		BodySprite->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		BodySprite->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		BodySprite->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		BodySprite->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		BodySprite->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		BodySprite->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		BodySprite->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
		BodySprite->SetCollisionResponseToChannel(ECC_Destructible, ECR_Ignore);
		// FRotator = rotation Y Z X
		BodySprite->SetWorldRotation(FQuat(FRotator(0, -90, -90)));
	}
	if (HeadSprite)
	{
		HeadSprite->SetupAttachment(Scene);
		HeadSprite->AlwaysLoadOnClient = true;
		HeadSprite->AlwaysLoadOnServer = true;
		HeadSprite->bOwnerNoSee = false;
		HeadSprite->bAffectDynamicIndirectLighting = true;
		HeadSprite->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		//HeadSprite->bGenerateOverlapEvents = false;
		HeadSprite->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		HeadSprite->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		HeadSprite->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		HeadSprite->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		HeadSprite->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		HeadSprite->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		HeadSprite->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
		HeadSprite->SetCollisionResponseToChannel(ECC_Destructible, ECR_Ignore);
		// FRotator = rotation Y Z X
		HeadSprite->SetWorldRotation(FQuat(FRotator(0, -90, -90)));
	}
	if (FootSprite)
	{
		FootSprite->SetupAttachment(Scene);
		FootSprite->AlwaysLoadOnClient = true;
		FootSprite->AlwaysLoadOnServer = true;
		FootSprite->bOwnerNoSee = false;
		FootSprite->bAffectDynamicIndirectLighting = true;
		FootSprite->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		//FootSprite->bGenerateOverlapEvents = false;
		FootSprite->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		FootSprite->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		FootSprite->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		FootSprite->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		FootSprite->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		FootSprite->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		FootSprite->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
		FootSprite->SetCollisionResponseToChannel(ECC_Destructible, ECR_Ignore);
		// FRotator = rotation Y Z X
		FootSprite->SetWorldRotation(FQuat(FRotator(0, -90, -90)));
	}

}

// Called when the game starts or when spawned
void AAbilityHint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAbilityHint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
#if WITH_EDITOR
void AAbilityHint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (MaxCastRange < MinCastRange)
	{
		MaxCastRange = MinCastRange;
	}
	UseDirectionSkill = false;
	UseRangeSkill = false;
	switch (SkillType)
	{
	case EAbilityHint::Direction:
	{
		UseDirectionSkill = true;
	}
	break;
	case EAbilityHint::Range:
	{
		UseRangeSkill = true;
	}
	break;
	}
	UpdateLength();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AAbilityHint::PostInitProperties()
{
	Super::PostInitProperties();
	UpdateLength();
}
#endif

void AAbilityHint::SetLength(float len)
{
	MaxCastRange = len;
	UpdateLength();
}

void AAbilityHint::UpdateLength()
{
	switch (SkillType)
	{
	case EAbilityHint::Direction:
	{
		if (FootSprite->GetSprite() && BodySprite->GetSprite() && HeadSprite->GetSprite())
		{
			float footwidth = FootSprite->GetSprite()->GetSourceSize().Y;
			float headwidth = HeadSprite->GetSprite()->GetSourceSize().Y;
			FVector2D size = BodySprite->GetSprite()->GetSourceSize();
			float scale = (MaxCastRange - headwidth - footwidth) / size.Y;
			BodySprite->SetWorldScale3D(FVector(1, 1, scale));
			BodySprite->SetRelativeLocation(FVector(footwidth, 0, 0));
			HeadSprite->SetRelativeLocation(FVector(MaxCastRange - headwidth, 0, 0));
		}
		else if (BodySprite->GetSprite() && HeadSprite->GetSprite())
		{
			float headwidth = HeadSprite->GetSprite()->GetSourceSize().Y;
			FVector2D size = BodySprite->GetSprite()->GetSourceSize();
			float scale = (MaxCastRange - headwidth) / size.Y;
			BodySprite->SetWorldScale3D(FVector(1, 1, scale));
			HeadSprite->SetRelativeLocation(FVector(MaxCastRange - headwidth, 0, 0));
		}
		else if (BodySprite->GetSprite())
		{
			FVector2D size = BodySprite->GetSprite()->GetSourceSize();
			float scale = (MaxCastRange) / size.Y;
			BodySprite->SetWorldScale3D(FVector(1, 1, scale));
		}
	}
	break;
	case EAbilityHint::Range:
	{
		if (BodySprite->GetSprite())
		{
			FVector2D size = BodySprite->GetSprite()->GetSourceSize();
			float scale = (SkillDiameter) / size.X;
			BodySprite->SetWorldScale3D(FVector(scale, 1, scale));
		}
	}
	break;
	}
}


void AAbilityHint::UpdatePos(FVector PlayerPos, FVector MousePos)
{
	FVector dir = MousePos - PlayerPos;
	dir.Z = 0;
	float len = dir.Size();
	dir.Normalize();
	switch (SkillType)
	{
	case EAbilityHint::Direction:
	{
		SetActorRotation(dir.Rotation());
		SetActorLocation(PlayerPos);
		if (len < MinCastRange)
		{
			len = MinCastRange;
		}
		else if (len > MaxCastRange)
		{
			len = MaxCastRange;
		}
		if (!IsFixdLength)
		{
			MaxCastRange = len;
			SkillPos = MousePos;
		}
		SkillPos = dir * len + PlayerPos;
	}
	break;
	case EAbilityHint::Range:
	{
		if (len < MinCastRange)
		{
			len = MinCastRange;
		}
		else if (len > MaxCastRange)
		{
			len = MaxCastRange;
		}
		FVector pos = dir * len + PlayerPos;
		SetActorLocation(pos);
	}
	break;
	}
	UpdateLength();
}

