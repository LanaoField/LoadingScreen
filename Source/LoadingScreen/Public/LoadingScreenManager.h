// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenManager.generated.h"



UCLASS(Config=Game)
class LOADINGSCREEN_API ULoadingScreenManager : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	static ULoadingScreenManager& Get();
	
	void Init();
	void Shutdown();

	UFUNCTION(BlueprintCallable, Category = LoadingScreen)
	static void PlayLoadingScreen(TSubclassOf<UUserWidget> WidgetClass, bool bIsAutoCompleteWhenLoadingCompletes = true, float InMinimumLoadingScreenDisplayTime = 0, float InDelayCompleteWhenPostLoadMap = 0.f);

	UFUNCTION(BlueprintCallable, Category = LoadingScreen)
	static void HideLoadingScreen();

protected:
	virtual void PreLoadMap(const FString& MapName);
	virtual void PostLoadMapWithWorld(class UWorld* LoadedWorld);

	FDelegateHandle PreLoadMapHandle;
	FDelegateHandle PostLoadMapWithWorldHandle;
	
	UPROPERTY(EditAnywhere, Category = LoadingScreen, Config)
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

	UPROPERTY(EditAnywhere, Category = LoadingScreen, Config)
	float MinimumLoadingScreenDisplayTime;

	UPROPERTY(EditAnywhere, Category = LoadingScreen, Config)
	float DelayCompleteWhenPostLoadMap;

	UPROPERTY(EditAnywhere, Category = LoadingScreen, Config)
	bool bAllowEngineTick;

private:
	UPROPERTY()
	float DelayComplete;
		
	static ULoadingScreenManager* Instance;
};
