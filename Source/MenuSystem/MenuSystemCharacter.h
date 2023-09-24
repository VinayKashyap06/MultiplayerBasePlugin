#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuSystemCharacter.generated.h"


UCLASS(config = Game)
class AMenuSystemCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

public:
	AMenuSystemCharacter();


protected:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay();

	//Input actions callbacks
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:

	//ptr to online session interface - includes creating destroying online sessions etc
	IOnlineSessionPtr OnlineSessionInterface;

protected:
	UFUNCTION(BlueprintCallable)
	void CreateGameSession(); //callable to invoke creating game session
	UFUNCTION(BlueprintCallable)
	void JoinGameSessions(); //callable to invoke joining game session

	UFUNCTION()
	void OnCreateSessionCompletedDelegate(FName SessionName, bool bWasSuccessfull); //bind to delegate
	UFUNCTION()
	void OnFindSessionsCompletedDelegate(bool bWasSuccessfull); // bind to delegate

	void OnJoinSessionCompletedDelegate(FName SessionName, EOnJoinSessionCompleteResult::Type Result); //bind to delegate

private:
	bool IsValidSessionInterface();

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FOnCreateSessionCompleteDelegate CreateSessionCompletedDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompletedDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
};

