// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AON2020.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, AON2020, "AON2020" );

FString GetNetString(AActor* actor)
{
	UWorld* World = actor->GetWorld(); //GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	FString Prefix;
	if (World)
	{
		if (World->WorldType == EWorldType::PIE)
		{
			switch (World->GetNetMode())
			{
			case NM_Client:
				Prefix = FString::Printf(TEXT("Client: "));
				break;
			case NM_DedicatedServer:
			case NM_ListenServer:
				Prefix = FString::Printf(TEXT("Server: "));
				break;
			case NM_Standalone:
				break;
			}
		}
	}
	return Prefix;
}


DEFINE_LOG_CATEGORY(LogAON2020)
 