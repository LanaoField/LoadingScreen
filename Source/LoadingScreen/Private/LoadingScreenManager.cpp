// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LoadingScreenManager.h"
#include "TickableObjectRenderThread.h"
#include "MoviePlayer.h"
#include "Async/Async.h"


class FStopMovieTask : public FTickableObjectRenderThread
{
public:
	static void Start(float Delay)
	{
		AsyncTask(ENamedThreads::ActualRenderingThread, [Delay]() {
			
			check(IsInRenderingThread());

			if (Instance == nullptr)
			{
				Instance = new FStopMovieTask();
				Instance->bStart = true;
				Instance->DelayStopTime = Delay;
			}
		});
	}

protected:
	virtual void Tick(float DeltaTime) override
	{
		check(IsInRenderingThread());

		if (bStart)
		{
			DelayStopTime -= DeltaTime;
			if (DelayStopTime <= 0.f)
			{
				GetMoviePlayer()->StopMovie();
				GetMoviePlayer()->ForceCompletion();

				delete Instance;
				Instance = nullptr;
			}
		}
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FStopMovieTask, STATGROUP_Tickables);
	}

	virtual bool IsTickable() const override
	{
		return true;
	}

private:
	bool bStart = false;

	float DelayStopTime = 0.f;

	static FStopMovieTask* Instance;
};

FStopMovieTask* FStopMovieTask::Instance = nullptr;


ULoadingScreenManager* ULoadingScreenManager::Instance = nullptr;

ULoadingScreenManager::ULoadingScreenManager(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MinimumLoadingScreenDisplayTime(-1.f)
	, DelayCompleteWhenPostLoadMap(0.f)
	, bAllowEngineTick(false)
{

}

ULoadingScreenManager& ULoadingScreenManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<ULoadingScreenManager>();
	}

	return *Instance;
}

void ULoadingScreenManager::Init()
{
	// Bind delegates
	PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ULoadingScreenManager::PreLoadMap);
	PostLoadMapWithWorldHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ULoadingScreenManager::PostLoadMapWithWorld);
}

void ULoadingScreenManager::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapWithWorldHandle);
}

void ULoadingScreenManager::PlayLoadingScreen(TSubclassOf<UUserWidget> WidgetClass, bool bIsAutoCompleteWhenLoadingCompletes, float InMinimumLoadingScreenDisplayTime, float InDelayCompleteWhenPostLoadMap)
{
	if(IsRunningDedicatedServer())
	{
		return;
	}

	if (GWorld != nullptr)
	{
		UGameInstance* GameInstance = GWorld->GetGameInstance();
		if (GameInstance != nullptr)
		{
			if (WidgetClass != nullptr)
			{
				UUserWidget* LoadingScreenWidget = UUserWidget::CreateWidgetInstance(*GameInstance, WidgetClass, WidgetClass->GetFName());
				if (LoadingScreenWidget != nullptr)
				{
					ULoadingScreenManager& LoadingScreenManager = ULoadingScreenManager::Get();
					if (InDelayCompleteWhenPostLoadMap > 0.f)
					{
						LoadingScreenManager.DelayComplete = InDelayCompleteWhenPostLoadMap;
					}

					FLoadingScreenAttributes LoadingScreenAttributes;
					LoadingScreenAttributes.bAllowEngineTick = LoadingScreenManager.bAllowEngineTick;
					LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = bIsAutoCompleteWhenLoadingCompletes;
					LoadingScreenAttributes.MinimumLoadingScreenDisplayTime = InMinimumLoadingScreenDisplayTime;
					LoadingScreenAttributes.WidgetLoadingScreen = LoadingScreenWidget->TakeWidget();

					GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
				}
			}

			GetMoviePlayer()->PlayMovie();
		}
	}
}

void ULoadingScreenManager::HideLoadingScreen()
{
	GetMoviePlayer()->StopMovie();
}

void ULoadingScreenManager::PreLoadMap(const FString& MapName)
{
	if (LoadingScreenWidgetClass != nullptr && !GetMoviePlayer()->IsStartupMoviePlaying())
	{
		PlayLoadingScreen(LoadingScreenWidgetClass, true, MinimumLoadingScreenDisplayTime, DelayCompleteWhenPostLoadMap);
	}
}

void ULoadingScreenManager::PostLoadMapWithWorld(class UWorld* LoadedWorld)
{
	if (DelayComplete > 0.f && GetMoviePlayer()->IsStartupMoviePlaying())
	{
		FStopMovieTask::Start(DelayComplete);

		DelayComplete = 0.f;
	}
}