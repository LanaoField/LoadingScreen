// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LoadingScreen.h"
#include "LoadingScreenManager.h"


#define LOCTEXT_NAMESPACE "FLoadingScreenModule"

void FLoadingScreenModule::StartupModule()
{
	ULoadingScreenManager::Get().Init();
}

void FLoadingScreenModule::ShutdownModule()
{
	ULoadingScreenManager::Get().Shutdown();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLoadingScreenModule, LoadingScreen)