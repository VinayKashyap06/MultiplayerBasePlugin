#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"


void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (GameState)
	{
		int32 numOfPlayers = GameState.Get()->PlayerArray.Num();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				60.0f,
				FColor::Cyan,
				FString::Printf(TEXT("Total players in game %d"), numOfPlayers)
			);

			auto PlayerState = NewPlayer->GetPlayerState<APlayerState>();

			if (PlayerState)
			{
				FString PlayerName = PlayerState->GetPlayerName();
				GEngine->AddOnScreenDebugMessage(
					-1,
					60.0f,
					FColor::Yellow,
					FString::Printf(TEXT("%s joined game "), *PlayerName)
				);
			}
		}
	}
}

void ALobbyGameMode::Logout(AController* Existing)
{
	Super::Logout(Existing);

	APlayerState* PlayerState = Existing->GetPlayerState<APlayerState>();

	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();
		if (GEngine) {

			GEngine->AddOnScreenDebugMessage(
				1,
				60.0f,
				FColor::Yellow,
				FString::Printf(TEXT("%s exited game "), *PlayerName)
			);
		}
	}

	int32 numOfPlayers = GameState.Get()->PlayerArray.Num();
	if (GEngine)
	{

		GEngine->AddOnScreenDebugMessage(
			-1,
			60.0f,
			FColor::Cyan,
			FString::Printf(TEXT("Players in game %d"), numOfPlayers - 1)
		);
	}
}
