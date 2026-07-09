// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/LostArkCharacter.h"
#include "UObject/ConstructorHelpers.h"

#include "Camera/CameraComponent.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

#include "Camera/CameraTypes.h"

#include "Materials/Material.h"
#include "Engine/World.h"


ALostArkCharacter::ALostArkCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Set Character Skeletal Mesh
	ConstructorHelpers::FObjectFinder<USkeletalMesh> 
		TempMesh(TEXT("SkeletalMesh'/Game/Characters/Mannequin_UE4/Meshes/SK_Mannequin.SK_Mannequin'"));

	if (TempMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempMesh.Object);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));
	}

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 1000.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Minimap 
	MinimapCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCapture"));
	MinimapCapture->SetupAttachment(RootComponent);

	MinimapCapture->SetRelativeLocation(FVector(0.f, 0.f, 1500.f));
	MinimapCapture->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	MinimapCapture->ProjectionType = ECameraProjectionMode::Orthographic;
	MinimapCapture->OrthoWidth = 3000.f;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;	
}

void ALostArkCharacter::BeginPlay()
{
	Super::BeginPlay();

	// PlayerController
	PlayerController = Cast<ALostArkPlayerController>(GetController());

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("LostArkPlayerController is null"));
		return;
	}

	IMC = PlayerController->GetIMC();

	if (!IMC)
	{
		UE_LOG(LogTemp, Warning, TEXT("IMC_LostArk is null"));
		return;
	}
}

void ALostArkCharacter::SetCameraArmLength(float length)
{
	if (!CameraBoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("CAMERABOOM IS NOT EXIST"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("ZOOM IS CLAMPING AGAIN"));
	CameraBoom->TargetArmLength = FMath::Clamp(length, 300.f, 1200.f);
}

void ALostArkCharacter::Attack(FHitResult point)
{
	UE_LOG(LogTemp, Warning, TEXT("Input Attack"));


}

void ALostArkCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ALostArkCharacter::DieProccess()
{
}