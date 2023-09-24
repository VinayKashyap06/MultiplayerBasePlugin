// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuUserWidget.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenuUserWidget::MenuSetup(int32 NumOfPublicConnections, FString matchType, FString lobbyPath)
{
	LobbyPath = FString::Printf(TEXT("%s?listen"), *lobbyPath);
	NumPublicConnections = NumOfPublicConnections;
	MatchType = matchType;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionSubsystem)
		{
			//dynamic binding
			MultiplayerSessionSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreatedSession);
			MultiplayerSessionSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartedSession);
			MultiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroyedSession);

			//non dynamic binding
			MultiplayerSessionSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this, &ThisClass::OnFindSession);
			MultiplayerSessionSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		}
	}
}

void UMenuUserWidget::OnJoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->FindSessions(100000);
	}

}

void UMenuUserWidget::OnHostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->CreateSession(4, FString("FreeForAll"));
	}
}

void UMenuUserWidget::MenuTearDown()
{
	RemoveFromParent();
	const UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

bool UMenuUserWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	//Buttons should already be bound to widgets
	//Bind button delegates
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::OnHostButtonClicked);
	}

	return true;
}

void UMenuUserWidget::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UMenuUserWidget::OnCreatedSession(bool bWasSuccessfull)
{
	if (bWasSuccessfull)
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				1,
				15.0f,
				FColor::Green,
				FString::Printf(TEXT("session create successfull"))
			);
		}
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(LobbyPath);
		}
	}
	else
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				1,
				15.0f,
				FColor::Red,
				FString::Printf(TEXT("session create failed"))
			);
		}
		HostButton->SetIsEnabled(bWasSuccessfull);
	}
}

void UMenuUserWidget::OnStartedSession(bool bWasSuccessfull)
{
	//TODO add session button and functionality, to be used only by host
}

void UMenuUserWidget::OnDestroyedSession(bool bWasSuccessfull)
{
	//TODO add quit session button and functionality
}

void UMenuUserWidget::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionSearchResults, bool bWasSuccessfull)
{
	if (!MultiplayerSessionSubsystem)
	{
		return;
	}

	for (auto result : SessionSearchResults)
	{
		FString ID = result.GetSessionIdStr();
		FString UserName = result.Session.OwningUserName;
		FString SettingsValue;
		result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Cyan,
				FString::Printf(TEXT("sessions found id= %s , username= %s"), *ID, *UserName)
			);
		}

		if (SettingsValue == MatchType)
		{
			MultiplayerSessionSubsystem->JoinSession(result);
		}
	}

	if (!bWasSuccessfull || SessionSearchResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenuUserWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSubSystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr OnlineSessionInterface = OnlineSubSystem->GetSessionInterface();

	FString Address;
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address)) {
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Cyan,
				FString::Printf(TEXT("connecting to address %s"), *Address)
			);
		}

		//since this is a client, and we are about to move, getting game instance lcoal player controller is prefered
		APlayerController* playerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (playerController)
		{
			playerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute); //since we are joining this is client travel
		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}
