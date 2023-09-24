#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuUserWidget.generated.h"

/**
 * This class is for User Widget that creates buttons and invokes delegates
 * TODO, add start session and quit session button
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenuUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumOfPublicConnections = 4, FString matchType = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("(/Game/ThirdPerson/Maps/Lobby?listen)")));

	UFUNCTION()
	void OnJoinButtonClicked();
	UFUNCTION()
	void OnHostButtonClicked();

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;

	//our subsystem to connect online
	class UMultiplayerSessionsSubsystem* MultiplayerSessionSubsystem;

	void MenuTearDown();
	int32 NumPublicConnections{ 4 };
	FString MatchType{ TEXT("FreeForAll") };
	FString LobbyPath{ TEXT("") };

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//functions to bind to delegates
	UFUNCTION()
	void OnCreatedSession(bool bWasSuccessfull);
	UFUNCTION()
	void OnStartedSession(bool bWasSuccessfull);
	UFUNCTION()
	void OnDestroyedSession(bool bWasSuccessfull);

	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionSearchResults, bool bWasSuccessfull);

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
};
