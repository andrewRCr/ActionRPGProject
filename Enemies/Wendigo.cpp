// © 2022 Andrew Creekmore 


#include "Wendigo.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "EnemyController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ActionRPGProject/Character/Main.h"
#include "ActionRPGProject/Character/MainPlayerController.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"



// sets default values
AWendigo::AWendigo()
{
	bIsAttackingSwipe = false;
	bIsAttackingHeadButt = false;
	bIsAttackingBite = false;

	WendigoAttackCounter = 0;

	MaxHealth = 150.f;
	Health = MaxHealth;

	MaxPoise = 50.f;
	Poise = MaxPoise;
}

// called when the game starts or when spawned
void AWendigo::BeginPlay()
{
	Super::BeginPlay();
}


void AWendigo::AttackEnd()
{
	// call parent function
	Super::AttackEnd();

	// reset Wendigo-specific flags
	bIsAttackingSwipe = false;
	bIsAttackingHeadButt = false;
	bIsAttackingBite = false;
}


void AWendigo::ResetAttackCounter()
{
	WendigoAttackCounter = 0;
	GetWorldTimerManager().ClearTimer(AttackTimer);
}