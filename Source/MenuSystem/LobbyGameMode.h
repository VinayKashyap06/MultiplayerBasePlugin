#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/// <summary>
/// Lobby level specific game mode to keep count of players entering lobby
/// </summary>
UCLASS()
class MENUSYSTEM_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual	void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Existing) override;
};