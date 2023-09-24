#include "MenuSystemCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"



//////////////////////////////////////////////////////////////////////////
// AMenuSystemCharacter
//creating delegates with initialize

AMenuSystemCharacter::AMenuSystemCharacter() :
	CreateSessionCompletedDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompletedDelegate)),
	FindSessionsCompletedDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsCompletedDelegate)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionCompletedDelegate))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 800.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void AMenuSystemCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void AMenuSystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Look);
	}
}

void AMenuSystemCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMenuSystemCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


void AMenuSystemCharacter::CreateGameSession()
{
	if (!IsValidSessionInterface())
	{
		return;
	}

	//destroy session if it already exists
	auto existingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (existingSession)
	{
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}

	//Bind callback to interface
	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompletedDelegate);

	//create new session
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings()); //settings to be created
	SessionSettings->bIsLANMatch = false;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->NumPublicConnections = 4;
	SessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineService);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
}

void AMenuSystemCharacter::JoinGameSessions()
{
	if (!IsValidSessionInterface())
		return;

	//Bind reference of our delegate
	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompletedDelegate);

	//create search
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());

}

bool AMenuSystemCharacter::IsValidSessionInterface()
{
	if (!OnlineSessionInterface)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			OnlineSessionInterface = Subsystem->GetSessionInterface();
		}
	}
	return OnlineSessionInterface.IsValid();
}

void AMenuSystemCharacter::OnCreateSessionCompletedDelegate(FName SessionName, bool bWasSuccessfull)
{
	if (bWasSuccessfull)
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				0,
				15.0f,
				FColor::Blue,
				FString::Printf(TEXT("Created session %s"), *SessionName.ToString())
			);
		}

		//travel to lobby, make sure seamless travel is on in game mode and transient map is set
		//since this is hosting, we call server travel because we are acting as server
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel("/Game/ThirdPerson/Maps/Lobby?listen");
		}
	}
	else {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				1,
				15.0f,
				FColor::Red,
				FString::Printf(TEXT("failed to create session %s"), *SessionName.ToString())
			);
		}
	}
}

void AMenuSystemCharacter::OnFindSessionsCompletedDelegate(bool bWasSuccessfull)
{
	if (!IsValidSessionInterface())
	{
		return;
	}

	for (auto result : SessionSearch->SearchResults)
	{
		FString ID = result.GetSessionIdStr();
		FString UserName = result.Session.OwningUserName;
		FString MatchType;
		result.Session.SessionSettings.Get(FName("MatchType"), MatchType);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Cyan,
				FString::Printf(TEXT("sessions found id= %s , username= %s"), *ID, *UserName)
			);
		}

		if (MatchType == FString("FreeForAll"))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.0f,
					FColor::Cyan,
					FString::Printf(TEXT("match type found"))
				);
			}

			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, result);
		}
	}
}

void AMenuSystemCharacter::OnJoinSessionCompletedDelegate(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!IsValidSessionInterface())
	{
		return;
	}

	FString Address;
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Cyan,
				FString::Printf(TEXT("connect string %s"), *Address)
			);
		}

		//since this is a client, and we are about to move, getting game instance lcoal player controller is prefered
		auto playerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (playerController)
		{
			playerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute); //since we are joining this is client travel
		}
	}
}


