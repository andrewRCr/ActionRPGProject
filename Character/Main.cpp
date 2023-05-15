// © 2022 Andrew Creekmore 


#include "Main.h"
#include "../ActionRPGProject.h"
#include "../Character/MainAnimInstance.h"
#include "../Components/InteractionComponent.h"
#include "../Components/InventoryComponent.h"
#include "../DebugMacros.h"
#include "../Enemies/Enemy.h"
#include "../Items/AccessoryItem.h"
#include "../Items/GearItem.h"
#include "../Items/ShieldItem.h"
#include "../Items/WeaponItem.h"
#include "../Weapons/MeleeDamage.h"
#include "../Weapons/MeleeWeapon.h"
#include "../Weapons/Shield.h"
#include "../Weapons/Weapon.h"
#include "../World/Pickup.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MainPlayerController.h"
#include "Materials/MaterialInstance.h"
#include "Misc/OutputDeviceNull.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "Main"

// sets default values
AMain::AMain()
{
 	// set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/**
	 *  setup components
	*/

	// create player inventory with 20 slots and 60 "kg" capacity
	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>("PlayerInventory");
	PlayerInventory->SetCapacity(20);
	PlayerInventory->SetWeightCapacity(60.0f);

	// create modular skeletal mesh
	// create equipment slot mapped meshes
	HairMesh = MainMeshes.Add(EEquippableSlot::EIS_Hair, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh")));
	ChestMesh = MainMeshes.Add(EEquippableSlot::EIS_Chest, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh")));
	CloakMesh = MainMeshes.Add(EEquippableSlot::EIS_Cloak, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CloakMesh")));
	Hand_R_Mesh = MainMeshes.Add(EEquippableSlot::EIS_Arm_R, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hand_R_Mesh")));
	Hand_L_Mesh = MainMeshes.Add(EEquippableSlot::EIS_Arm_L, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hand_L_Mesh")));
	LegsMesh = MainMeshes.Add(EEquippableSlot::EIS_Legs, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh")));
	FeetMesh = MainMeshes.Add(EEquippableSlot::EIS_Shoes, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh")));
	Weapon_R_Mesh = MainMeshes.Add(EEquippableSlot::EIS_PrimaryWeapon, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon_R_Mesh")));
	Weapon_L_Mesh = MainMeshes.Add(EEquippableSlot::EIS_SecondaryWeapon, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon_L_Mesh")));

	// tell all mapped (i.e., corresponding to equip slots) body meshes to use head mesh for animations
	for (auto& PlayerMesh : MainMeshes)
	{
		USkeletalMeshComponent* MeshComponent = PlayerMesh.Value;
		MeshComponent->SetupAttachment(GetMesh());
		MeshComponent->SetMasterPoseComponent(GetMesh());
	}

	// head (unmapped) / helmet (attached to HAIR socket) set up separately
	MainMeshes.Add(EEquippableSlot::EIS_Head, GetMesh());

	HelmetMesh = MainMeshes.Add(EEquippableSlot::EIS_Helmet, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh")));
	HelmetMesh->SetupAttachment(GetMesh(), FName("HAIR"));

	// create Camera Boom (pulls towards the player if there's a collision) + set its defaults
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 220.f; // camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // rotate arm based on controller

	// create second Camera Boom (for smoother adjustment when collision detected) + set its defaults
	CameraBoom2 = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom2"));
	CameraBoom2->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraBoom2->TargetArmLength = 220.f; // camera follows at this distance
	CameraBoom2->bUsePawnControlRotation = true; // rotate arm based on controller

	// create Follow Camera (i.e., principal gameplay camera) + attach to second boom + set defaults
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom2, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// offset camera to player character's right shoulder
	CameraBoom->SocketOffset = FVector(0.f, 75.f, 50.f);

	// set size for player character collision capsule
	GetCapsuleComponent()->InitCapsuleSize(25.f, 69.f);

	// set turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// don't rotate character when controller rotates, only the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// set defaults for interaction
	InteractionCheckFrequency = 0.25f;
	InteractionCheckDistance = 1000.0f;
	bInteractableFoundOnLastCheck = false;

	/**
	 *  player stats: movement, stamina defaults / costs
	 */

	GetCharacterMovement()->bOrientRotationToMovement = true; // character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.00001f;

	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	WalkingSpeed = 166.666672;
	RunningSpeed = 375.f;
	SprintingSpeed = 625.f;
	CrouchedSpeed = WalkingSpeed;
	IdleSpeed = 0.f;
	EvadeDistance = 400.0f;

	// defaults for player character interpolation relative to enemies
	RInterpToEnemySpeed = 15.f;
	VInterpToEnemySpeed = 1.f;
	VInterpToEnemySpeedUnarmed = 2.f;
	TooCloseBackupVInterpSpeed = 1.f;
	TooFarToVInterpThreshold = 500.f;
	TooCloseToVInterpThreshold = 150.f;
	TooCloseToVInterpThresholdUnarmed = 100.f;

	// default delay upon death (to allow showing death screen UI, music, etc)
	RespawnDelay = 5.0f;

	MaxStamina = 100.0f;
	Stamina = MaxStamina;
	StaminaDrainRate = 20.f;
	DefaultStaminaRegenRate = 45.f;
	CurrentStaminaRegenRate = DefaultStaminaRegenRate;
	BlockingStaminaRegenRateModifier = 0.25f; // how much actively blocking modifies stamina regen rate by

	StaminaRegenDelayCounter = 0.0f;
	StaminaRegenDelayRate = 1.f;
	RegenDelay = 0.0f;

	BlockingStaminaRegenDelayAmount = 1.0f;
	AttackingStaminaRegenDelayAmount = 1.0f;
	ExhaustedStaminaRegenDelayAmount = 3.0f;

	MinSprintStamina = 25.f;
	JumpStaminaCost = 20.f;
	DodgeStaminaCost = 15.f;
	RollStaminaCost = 25.f;

	UnarmedLightAttackStaminaCost = 15.f;
	UnarmedHeavyAttackStaminaCost = 25.f;

	HitSoundToPlay = 0; // int for cycling hit sound FX

	/**
	 *  movement modifiers
	 */
	
	bIsMovementLocked = false;
	bIsMoveInputIgnored = false;
	bIsIdle = true;
	bIsWalking = false;
	bIsCrouching = false;
	bCanEvade = true;
	bCanMove = true;
	bCanJump = true;
	bInAir = false;
	bIsDodging = false;
	bIsRolling = false;
	bEvading = false;
	bActiveIFrames = false;
	bIsEquipping = false;
	bInterpToEnemy = false;
	bCanRegenStamina = true;

	/**
	 *  player stats: health, damage and combat
	 */

	MaxHealth = 125.f;
	Health = MaxHealth;
	DelayedHealthReportingValue = MaxHealth;
	DelayedHealthBarValue = MaxHealth;
	DelayedHealthBarDrainRate = 20.0f;
	TakeDamageDelay = 0.5f;

	TotalHealthPotionCapacity = 0;
	AvailableHealthPotions = 0;
	HealthPotionRestoreAmount = 50.f;

	// default damage values when no weapon equipped
	UnarmedLightAttackDamage = 10.f;
	UnarmedHeavyAttackDamage = 17.5f;

	/**
	 *  combat modifiers
	 */

	bHasWeaponEquipped = false;
	bHasWeaponDrawn = false;
	bCheatsOn = false;
	bHasCombatTarget = false;
	bInSoftLockRange = false;
	bAttacking = false;
	bHeavyAttacking = false;
	bChargingHeavyAttack = false;
	bShieldAttacking = false;
	bCanTakeDamage = true;
	bBlockAttemptSucceeded = false;
	bPerformingExecution = false;
	bPlayingUnarmedAttackAnim = false;
	bPlayingUnarmedRunningLightAttackAnim = false;
	bPlayingUnarmedRunningHeavyAttackAnim = false;

	/**
	 *   control modifiers
	 */

	bShiftKeyDown = false;
	bLightAttackDown = false;
	bBlockDown = false;
	bInteractKeyDown = false;
	bHeavyAttackDown = false;
	bDodgePressed = false;
	bUsingPotion = false;

	/**
	 *  animation modifiers
	 */

	 LightAttackAnimationCounter = 0; // int32
	 HeavyAttackAnimationCounter = 0; // int32
	 LightForwardAnimationCounter = 0; // int32

	 ShieldSpawnDelay = 1.f;

	/**
	 *   currency / inventory related defaults
	 */

	 Coins = 0; // default placeholder - may or may not utilize
	 bShouldPlayGearEquipUnequipSounds = true;
	 NoNeedToUsePotionSoundVolumeMultiplier = 1.f; // adjustable volume for 'error' indicator when attempting to heal at full health
}

// called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	// when the player spawns in, they have no items equipped. cache these (so if a player unequips an item we can reset back to base skin meshes)
	for (auto& PlayerMesh : MainMeshes)
	{ NakedMeshes.Add(PlayerMesh.Key, PlayerMesh.Value->SkeletalMesh); }
}


void AMain::Restart()
{
	Super::Restart();

	if (AMainPlayerController* PlayerController = Cast<AMainPlayerController>(GetController()))
	{ PlayerController->ShowInGameUI(); }
}


void AMain::OnLootSourceOwnerDestroyed(AActor* DestroyedActor)
{	
	// remove loot source
	if (LootSource && (DestroyedActor == LootSource->GetOwner()))
	{ SetLootSource(nullptr); }
}


void AMain::OnRep_LootSource()
{
	// display or remove the looting menu
	if (AMainPlayerController* PlayerController = Cast<AMainPlayerController>(GetController()))
	{
		if (LootSource)
		{ PlayerController->ShowLootMenu(LootSource); }

		else
		{ PlayerController->HideLootMenu(); }
	}
}


// called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	//  check for interactables in front of player character - optimization (so not checking every single frame)
	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{ PerformInteractionCheck(); }

	/**
	*   SOFT automatic lock-on + vacuum interpolation towards enemies
	*/

	// unlocked (i.e., not actively using HARD target lock system), soft lock-on rotation interpolation to closest / most-facing enemy
	if (!bLockedOn && bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, RInterpToEnemySpeed);

		SetActorRotation(InterpRotation);
	}

	// vacuum to enemy slightly during attacks, locked on or not, if in close range - moreso (higher speed) when unarmed than armed
	if (bInterpToEnemy && CombatTarget && bInSoftLockRange)
	{
		float Distance = FVector::Distance(GetActorLocation(), CombatTarget->GetActorLocation());
		float CloseThresholdToUse = bHasWeaponDrawn ? TooCloseToVInterpThreshold : TooCloseToVInterpThresholdUnarmed;

		// disable root motion if right up on enemy, to keep from it causing character to move into them
		if (Distance < CloseThresholdToUse)
		{
			DisableAttackRootMotionBP();

			// back player up slightly when attacking if *really* up on the enemy close
			float BackupThresholdToUse = bHasWeaponDrawn ? 100.f : 75.f;
			if (Distance < BackupThresholdToUse)
			{
				FVector BackupLocation = GetActorLocation() + (GetActorForwardVector() * -100.f);
				FVector InterpLocation = FMath::VInterpTo(GetActorLocation(), BackupLocation, DeltaTime, TooCloseBackupVInterpSpeed);
				SetActorLocation(InterpLocation);
			}
		}

		// interp to enemy location (not for heavy armed/unarmed attacks / running armed attacks)
		else if (TooFarToVInterpThreshold > Distance && Distance > CloseThresholdToUse && !bHeavyAttacking)
		{
			if (bHasWeaponDrawn)
			{
				if (!EquippedWeapon->bPlayingRunningLightAttackAnim && !EquippedWeapon->bPlayingRunningHeavyAttackAnim)
				{
					FVector InterpLocation = FMath::VInterpTo(GetActorLocation(), CombatTarget->GetActorLocation(), DeltaTime, VInterpToEnemySpeed);
					SetActorLocation(InterpLocation);
				}
			}
			else
			{
				FVector InterpLocation = FMath::VInterpTo(GetActorLocation(), CombatTarget->GetActorLocation(), DeltaTime, VInterpToEnemySpeedUnarmed);
				SetActorLocation(InterpLocation);
			}
		}
	}

	/**
	*   stamina management
	*/

	// determine how much stamina to drain/restore per frame
	float DeltaStaminaDrain = StaminaDrainRate * DeltaTime;
	float DeltaStaminaRegen = CurrentStaminaRegenRate * DeltaTime;

	// determine how much red "delayed" health to drain after delay until meeting regular current health value (health bar effect)
	float DeltaDelayedHealthBarDrain = DelayedHealthBarDrainRate * DeltaTime;

	if (DelayedHealthBarValue > DelayedHealthReportingValue)
	{ DelayedHealthBarValue -= DeltaDelayedHealthBarDrain; }

	else if (DelayedHealthReportingValue > DelayedHealthBarValue)
	{ DelayedHealthBarValue = DelayedHealthReportingValue; }

	// manage StaminaStatus states
	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
	// default, "normal" stamina state

		bCanJump = true;

		if (Stamina < MinSprintStamina)
		{ SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);}

		// if sprinting
		if (MovementStatus == EMovementStatus::EMS_Sprinting && GetCharacterMovement()->MaxWalkSpeed == SprintingSpeed)
		{
			// if threshold reached, drop stamina status
			if (Stamina - DeltaStaminaDrain < MinSprintStamina)
			{ SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum); }

			if (!bCheatsOn)
			{
				// if not idle, continue draining stamina
				if (GetLastMovementInputVector() != FVector(0.f, 0.f, 0.f))
				{ Stamina -= DeltaStaminaDrain; }

				// if conditions met, regain stamina
				else if (((GetCharacterMovement()->IsFalling() == false) && (!bAttacking) && (!bIsDodging) && (!bIsRolling) && (bCanRegenStamina)))
				{ Stamina += DeltaStaminaRegen; }
			}
		}

		else // shift key is up / not sprinting successfully
		{
			if (Stamina + DeltaStaminaRegen >= MaxStamina)
			{ Stamina = MaxStamina; }

			else
			{
				// if conditions met, regain stamina
				if ((GetCharacterMovement()->IsFalling() == false) && (!bAttacking) && (!bIsDodging) && (!bIsRolling) && (bCanRegenStamina))
				{ Stamina += DeltaStaminaRegen; }
			}

			// set MovementStatus according to control modifier flags
			if (bIsWalking == false && bIsCrouching == false && (MovementStatus != EMovementStatus::EMS_Dead))
			{ SetMovementStatus(EMovementStatus::EMS_Normal); }
			
			else if (bIsCrouching == true)
			{ SetMovementStatus(EMovementStatus::EMS_Crouched); }
			
			else if (bIsWalking == true)
			{ SetMovementStatus(EMovementStatus::EMS_Walking); }
		}

		break;

	case EStaminaStatus::ESS_BelowMinimum:
	// low stamina (below threshold) state
		
		bCanJump = false;

		if (Stamina <= 0)
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina = 0;
		}

		if (bShiftKeyDown) // if attempting sprint while BelowMinimum
		{
			if (Stamina - DeltaStaminaDrain <= 0.f) // fully drained
			{
				// bottom out stamina to 0 + switch to Exhausted state if not idle
				if (GetLastMovementInputVector() != FVector(0.f, 0.f, 0.f))
				{
					SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
					Stamina = 0;
				}
			}

			else // not yet fully drained
			{
				if (!bCheatsOn)
				{
					// if not idle, drain stamina
					if (GetLastMovementInputVector() != FVector(0.f, 0.f, 0.f))
					{ Stamina -= DeltaStaminaDrain; }		
				}
			}
		}
		else // shift key is up; recover
		{
			if (MovementStatus != EMovementStatus::EMS_Dead)
			{
				if (Stamina + DeltaStaminaRegen >= MinSprintStamina)
				{ SetStaminaStatus(EStaminaStatus::ESS_Normal); }

				if ((!bAttacking) && (!bIsDodging) && (!bIsRolling) && (bCanRegenStamina))
				{ Stamina += DeltaStaminaRegen; }
			}
		}

		break;

	case EStaminaStatus::ESS_Exhausted:
	// completely drained state; transition state between BelowMinimum and ExhaustedRecovering

		bCanJump = false;

		// setup for recovery by switching states
		SetMovementStatus(EMovementStatus::EMS_Normal);
		SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);

		// ...but don't actually begin stamina recovery immeditately
		bCanRegenStamina = false;
		GetWorldTimerManager().SetTimer(StaminaRegenDelayTimer, this, &AMain::ResetCanRegenStamina, AttackingStaminaRegenDelayAmount);

		break;

	case EStaminaStatus::ESS_ExhaustedRecovering:
	// recovering back to Normal state from Exhausted - short window of vulnerability

		StaminaRegenDelayCounter = 0.0f;

		if ((!bAttacking) && (!bIsDodging) && (!bIsRolling) && (bCanRegenStamina))
		{ Stamina += DeltaStaminaRegen; }

		if ((Stamina >= MinSprintStamina) && (MovementStatus != EMovementStatus::EMS_Dead))
		{ SetStaminaStatus(EStaminaStatus::ESS_Normal); }

		break;

	default:
		;
	}
}


// check for interactable object in range
void AMain::PerformInteractionCheck()
{
	if (GetController() == nullptr)
	{ return; }

	// log time to help determine when to check next after this
	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	// setup trace parameters
	FVector PlayerEyesLoc;
	FRotator PlayerEyesRot;
	PlayerEyesLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	PlayerEyesRot = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorRotation();
	
	FVector SweepStart = PlayerEyesLoc;
	FVector SweepEnd = (PlayerEyesRot.Vector() * InteractionCheckDistance) + SweepStart;
	TArray<FHitResult> OutResults;
	FCollisionShape PlayerCloseCapsule = FCollisionShape::MakeCapsule(50.0f, 50.0f);

	// perform trace
	bool isHit = (GetWorld()->SweepMultiByChannel(OutResults, SweepStart, SweepEnd, FQuat::Identity, ECC_Visibility, PlayerCloseCapsule));
	
	// validate hit
	if (isHit)
	{
		for (auto& Hit : OutResults)
		{
			// verify hit actor is an interactable
			if (Hit.GetActor() != nullptr)
			{
				if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(Hit.GetActor()->FindComponentByClass(UInteractionComponent::StaticClass())))
				{
					float Distance = (SweepStart - InteractionComponent->GetComponentLocation()).Size();

					// success; interactable found
					if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance)
					{ FoundNewInteractable(InteractionComponent); }

					// too far away from interactable
					else if (Distance > InteractionComponent->InteractionDistance && GetInteractable())
					{ CouldntFindInteractable(); }

					return;
				}
			}
		}
	}

	CouldntFindInteractable();
}


void AMain::CouldntFindInteractable()
{
	// we've lost focus on an interactable; clear the timer
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{ GetWorldTimerManager().ClearTimer(TimerHandle_Interact); }

	// tell the interactable we've stopped focusing on it, and clear the current interactable
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld)
		{ EndInteract(); }
	}

	InteractionData.ViewedInteractionComponent = nullptr;
	bInteractableFoundOnLastCheck = false;
}



void AMain::FoundNewInteractable(UInteractionComponent* Interactable)
{
	// cancel any current interaction
	EndInteract();

	if (UInteractionComponent* OldInteractable = GetInteractable())
	{ OldInteractable->EndFocus(this); }

	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(this);

	bInteractableFoundOnLastCheck = true;
}


void AMain::BeginInteract()
{
	PerformInteractionCheck();

	InteractionData.bInteractHeld = true;

	// if have interactable
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		// has interaction component
		Interactable->BeginInteract(this);

		if (FMath::IsNearlyZero(Interactable->InteractionTime))
		{ Interact(); } // instant interact (on press)

		else // hold to interact
		{ GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &AMain::Interact, Interactable->InteractionTime, false); }
	}
}

void AMain::EndInteract()
{
	InteractionData.bInteractHeld = false;
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	// if have interactable
	if (UInteractionComponent* Interactable = GetInteractable())
	{ Interactable->EndInteract(this); }
}

void AMain::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
	{ Interactable->Interact(this); }
}

bool AMain::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float AMain::GetRemainingInteractTime()
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}


void AMain::ResetBlockSuccessTracking()
{
	bBlockAttemptSucceeded = false;
}


// called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	// mouse camera movement
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	// keyboard camera movement
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

	// MOVEMENT / EVASION - now handled in blueprint
	//PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	//PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);
	//PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AMain::Dodge);
	//PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &AMain::Roll);

	// CROUCHING / JUMPING - no longer featured in game
	//PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMain::CrouchStart);
	//PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMain::CrouchStop);
	//PlayerInputComponent->BindAction("CrouchToggle", IE_Pressed, this, &AMain::CrouchToggle);
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::JumpStart);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &AMain::WalkToggle);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMain::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMain::EndInteract);

	PlayerInputComponent->BindAction("Light_Attack", IE_Pressed, this, &AMain::LightAttackDown);
	PlayerInputComponent->BindAction("Light_Attack", IE_Released, this, &AMain::LightAttackUp);

	PlayerInputComponent->BindAction("Heavy_Attack", IE_Pressed, this, &AMain::HeavyAttackDown);
	PlayerInputComponent->BindAction("Heavy_Attack", IE_Released, this, &AMain::HeavyAttackUp);

	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AMain::BlockDown);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &AMain::BlockUp);

	PlayerInputComponent->BindAction("Use_Potion", IE_Pressed, this, &AMain::UsePotion);

	// CHEATS TOGGLE - disabled :D
	//PlayerInputComponent->BindAction("DeveloperCheatsToggle", IE_Pressed, this, &AMain::DeveloperCheatsToggle);
}


void AMain::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;

	if (bCanJump == true) // if can jump, can sprint (i.e., not exhausted)
	{
		if (StaminaStatus != EStaminaStatus::ESS_Exhausted)
		{
			SetMovementStatus(EMovementStatus::EMS_Sprinting);
			ShowHUDCall(); // always show HUD if stamina is draining
		}
	}
}


void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;

	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (bIsWalking == true) 
		{ SetMovementStatus(EMovementStatus::EMS_Walking); }
		
		else
		{ SetMovementStatus(EMovementStatus::EMS_Normal); }
	}
}

// set flags; actual triggering of StartAttack() now handled blueprint-side
void AMain::LightAttackDown()
{
	bLightAttackDown = true;
}


void AMain::LightAttackUp()
{
	bLightAttackDown = false;
	StopAttack();
}

// set flags; actual triggering of StartAttack() now handled blueprint-side
void AMain::HeavyAttackDown()
{
	bHeavyAttackDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) return;
	bHeavyAttacking = true;
}


void AMain::HeavyAttackUp()
{
	if (bChargingHeavyAttack)
	{
		EquippedWeapon->ReleaseChargedAttackBP();
		bChargingHeavyAttack = false;
	}

	bHeavyAttackDown = false;
	StopAttack();
}


void AMain::BlockDown()
{
	bBlockDown = true;
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	// if shield equipped, block
	if (EquippedShield && CanBlock())
	{ StartBlock(); }
}


void AMain::BlockUp()
{
	bBlockDown = false;
	StopBlock();
}


void AMain::InteractKeyDown()
{
	bInteractKeyDown = true;
}


void AMain::InteractKeyUp()
{
	bInteractKeyDown = false;
}


void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;

	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{ GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed; }
	
	else if (MovementStatus == EMovementStatus::EMS_Walking)
	{ GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed; }

	else if (MovementStatus == EMovementStatus::EMS_Normal)
	{ GetCharacterMovement()->MaxWalkSpeed = RunningSpeed; }

	else if (MovementStatus == EMovementStatus::EMS_Crouched)
	{ GetCharacterMovement()->MaxWalkSpeed = CrouchedSpeed; }

	else if (MovementStatus == EMovementStatus::EMS_Idle)
	{ GetCharacterMovement()->MaxWalkSpeed = IdleSpeed; }
}


// wrapper for Jump()
void AMain::JumpStart()
{
	if ((bCanJump) && (!bAttacking) && (!bIsDodging) && (!bIsRolling) && (MovementStatus != EMovementStatus::EMS_Dead)) // i.e., not in Exhausted stamina state
	{
		if (Stamina -= JumpStaminaCost > 0)
		{
			// keep character from rotating while in air during jump
			GetCharacterMovement()->bOrientRotationToMovement = false;

			if (!bCheatsOn)
			{ Stamina -= JumpStaminaCost; }

			Jump();
		}
	}
}


void AMain::WalkStart()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		SetMovementStatus(EMovementStatus::EMS_Walking);
		bIsWalking = true;
	}
}


void AMain::WalkStop()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		SetMovementStatus(EMovementStatus::EMS_Normal);
		bIsWalking = false;
	}
}


void AMain::WalkToggle()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (GetCharacterMovement()->MaxWalkSpeed == RunningSpeed)
		{
			SetMovementStatus(EMovementStatus::EMS_Walking);
			bIsWalking = true;
		}
		else
		{
			SetMovementStatus(EMovementStatus::EMS_Normal);
			bIsWalking = false;
		}
	}
}


void AMain::CrouchStart()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{ SetMovementStatus(EMovementStatus::EMS_Crouched); }
}

void AMain::CrouchStop()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (bIsWalking == true)
		{ SetMovementStatus(EMovementStatus::EMS_Walking); }
		
		else
		{ SetMovementStatus(EMovementStatus::EMS_Normal); }
	}
}

void AMain::CrouchToggle()
{
	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (!bIsCrouching)
		{
			SetMovementStatus(EMovementStatus::EMS_Crouched);
			bIsCrouching = true;
		}

		else
		{
			if (bIsWalking)
			{ SetMovementStatus(EMovementStatus::EMS_Walking); }
			
			else
			{ SetMovementStatus(EMovementStatus::EMS_Normal); }
			bIsCrouching = false;
		}
	}
}


void AMain::ResetCanRegenStamina()
{
	bCanRegenStamina = true;
}


void AMain::PlayDodgeSound()
{
	if (DodgeSound)
	{ UGameplayStatics::PlaySound2D(this, DodgeSound, 2.0f); }
}

FVector AMain::GetMovementDirection()
{
	float dodgeDirLeftRight = GetInputAxisValue(FName("MoveRight"));
	float dodgeDirForwardBackward = GetInputAxisValue(FName("MoveForward"));
	FVector Direction;

	// left
	if ((dodgeDirLeftRight == -1) && (dodgeDirForwardBackward == 0))
	{ Direction = FVector(0, -1, 0); }

	// forward left
	else if ((dodgeDirLeftRight == -1) && (dodgeDirForwardBackward == 1))
	{ Direction = FVector(1, -1, 0);}

	// backward left
	else if ((dodgeDirLeftRight == -1) && (dodgeDirForwardBackward == -1))
	{ Direction = FVector(-1, -1, 0); }
	
	// right
	else if ((dodgeDirLeftRight == 1) && (dodgeDirForwardBackward == 0))
	{ Direction = FVector(0, 1, 0); }

	// forward right
	else if ((dodgeDirLeftRight == 1) && (dodgeDirForwardBackward == 1))
	{ Direction = FVector(1, 1, 0); }

	// backward right
	else if ((dodgeDirLeftRight == 1) && (dodgeDirForwardBackward == -1))
	{ Direction = FVector(-1, 1, 0); }

	// forward
	else if ((dodgeDirLeftRight == 0) && (dodgeDirForwardBackward == 1))
	{ Direction = FVector(1, 0, 0); }

	// backward
	else if ((dodgeDirLeftRight == 0) && (dodgeDirForwardBackward == -1))
	{ Direction = FVector(-1, 0, 0); }

	return Direction;
}


void AMain::DeveloperCheatsToggle()
{
	if (!bCheatsOn)
	{ bCheatsOn = true; }

	else
	{ bCheatsOn = false; }
}


/**
 *   inventory / items
 */

void AMain::SetLootSource(class UInventoryComponent* NewLootSource)
{
	// if item we're looting gets destroyed, remove loot screen UI
	if (NewLootSource && NewLootSource->GetOwner())
	{ NewLootSource->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &AMain::OnLootSourceOwnerDestroyed); }

	LootSource = NewLootSource;
	OnRep_LootSource();
}


bool AMain::IsLooting() const
{
	// if we have a valid loot source, we must be looting
	return LootSource != nullptr;
}


void AMain::LootItem(class UItem* ItemToGive)
{
	if (PlayerInventory && LootSource && ItemToGive && LootSource->HasItem(ItemToGive->GetClass(), ItemToGive->GetQuantity()))
	{
		const FItemAddResult AddResult = PlayerInventory->TryAddItem(ItemToGive);

		// if successful, remove from source after adding to player inventory
		if (AddResult.ActualAmountGiven > 0)
		{ LootSource->ConsumeItem(ItemToGive, AddResult.ActualAmountGiven); }

		else
		{
			// notify player why they couldn't loot the item
			if (AMainPlayerController* PlayerController = Cast<AMainPlayerController>(GetController()))
			{ PlayerController->ClientShowNotification(AddResult.ErrorText); }
		}
	}
}


bool AMain::EquipItem(class UEquippableItem* Item)
{
	for (int32 i = 0; i < Item->Slots.Num(); ++i)
	{
		EquippedItems.Add(Item->Slots[i], Item);
		OnEquippedItemsChanged.Broadcast(Item->Slots[i], Item);
	}

	return true;
}


bool AMain::UnequipItem(class UEquippableItem* Item)
{
	if (Item)
	{
		for (int32 i = 0; i < Item->Slots.Num(); ++i)
		{
			if (EquippedItems.Contains(Item->Slots[i]))
			{
				if (Item == *EquippedItems.Find(Item->Slots[i]))
				{
					EquippedItems.Remove(Item->Slots[i]);
					OnEquippedItemsChanged.Broadcast(Item->Slots[i], nullptr);
					
				}
			}
		}

		return true;
	}

	return false;
}


void AMain::EquipGear(class UGearItem* Gear)
{
	for (int32 i = 0; i < Gear->Meshes.Num(); ++i)
	{
		if (USkeletalMeshComponent* GearMesh = *MainMeshes.Find(Gear->Slots[i]))
		{ GearMesh->SetSkeletalMesh(Gear->Meshes[i]); }
	}

	if (Gear->EquipSound && bShouldPlayGearEquipUnequipSounds)
	{ PlayGearEquipSFX(true, Gear->EquipSound); }
}


void AMain::UnequipGear(const EEquippableSlot Slot, class UGearItem* Gear)
{
	if (USkeletalMeshComponent* EquippableMesh = *MainMeshes.Find(Slot))
	{
		if (USkeletalMesh* BodyMesh = *NakedMeshes.Find(Slot))
		{ EquippableMesh->SetSkeletalMesh(BodyMesh); }

		else // some gear (e.g., accessories) has no "naked" mesh
		{ EquippableMesh->SetSkeletalMesh(nullptr);	}

		if (Gear->UnequipSound && bShouldPlayGearEquipUnequipSounds) // don't play unequip sound if another one is being equipped in its place 
		{ PlayGearEquipSFX(false, Gear->UnequipSound); }
	}
}


void AMain::EquipWeapon(class UWeaponItem* WeaponItem)
{
	if (WeaponItem && WeaponItem->WeaponClass)
	{
		if (EquippedWeapon)
		{ UnequipWeapon(); }

		UnarmedAttackEnd();
		OnUnarmedAttackAnimationEnd();

		bIsEquipping = true;

		// spawn weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = SpawnParams.Instigator = this;

		if (AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponItem->WeaponClass, SpawnParams))
		{
			Weapon->Item = WeaponItem;
			EquippedWeapon = Weapon;
			Weapon->OnEquip();
		}
	}
}


void AMain::UnequipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttackEnd();
		EquippedWeapon->OnAttackAnimationEnd();

		EquippedWeapon->OnUnequip();
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
		bHasWeaponDrawn = false;
	}
}


void AMain::EquipShield(class UShieldItem* ShieldItem)
{
	if (ShieldItem && ShieldItem->ShieldClass && !bAttacking && !bIsDodging && !bIsRolling && !bEvading)
	{
		if (EquippedShield)
		{
			EquippedShield->bShouldPlayUnequipSound = false;
			UnequipShield();
		}

		UnarmedAttackEnd();
		OnUnarmedAttackAnimationEnd();
		bCanEvade = false;
		bIsEquipping = true;
		EquippedShieldItem = ShieldItem;

		if (ShieldEquipAnim)
		{ PlayAnimMontage(ShieldEquipAnim, 1.f); }

		GetWorldTimerManager().SetTimer(TimerHandle_SpawnShield, this, &AMain::SpawnShieldAndFinishEquip, ShieldSpawnDelay);
	}
}


void AMain::SpawnShieldAndFinishEquip()
{
	if (EquippedShield) // another shield already equipped? clear it out before spawning another
	{
		DespawnShieldAndFinishUnequip();
	}

	// spawn shield
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = SpawnParams.Instigator = this;

	if (AShield* Shield = GetWorld()->SpawnActor<AShield>(EquippedShieldItem->ShieldClass, SpawnParams))
	{
		Shield->Item = EquippedShieldItem;
		EquippedShield = Shield;
		Shield->OnEquip();
	}
}



void AMain::UnequipShield()
{
	if (EquippedShield && !bAttacking && !bIsDodging && !bIsRolling && !bEvading)
	{
		bCanEvade = false;

		if (ShieldUnequipAnim)
		{ PlayAnimMontage(ShieldUnequipAnim, 1.f); }

		if (!bIsEquipping) // regular unequip, not result of equipping another shield
		{ GetWorldTimerManager().SetTimer(TimerHandle_SpawnShield, this, &AMain::DespawnShieldAndFinishUnequip, ShieldSpawnDelay); }
	}
}


void AMain::DespawnShieldAndFinishUnequip()
{
	EquippedShield->OnUnequip();
	EquippedShield->Destroy();
	EquippedShield = nullptr;
}


void AMain::EquipAccessory(class UAccessoryItem* AccessoryItem)
{
	// placeholder - may or may not implement
}

void AMain::UnequipAccessory(class UAccessoryItem* AccessoryItem)
{
	// placeholder - may or may not implement
}

class USkeletalMeshComponent* AMain::GetSlotSkeletalMeshComponent(const EEquippableSlot Slot)
{
	if (MainMeshes.Contains(Slot))
	{ return *MainMeshes.Find(Slot); }

	return nullptr;
}


void AMain::UseItem(class UItem* Item)
{
	if (Item)
	{
		// can't use an item you don't have
		if (PlayerInventory && !PlayerInventory->FindItem(Item))
		{ return; }

		Item->Use(this);
	}
}


void AMain::DropItem(class UItem* Item, const int32 Quantity)
{
	if (PlayerInventory && Item && PlayerInventory->FindItem(Item))
	{
		const int32 ItemQuantity = Item->GetQuantity();
		const int32 DroppedQuantity = PlayerInventory->ConsumeItem(Item, Quantity);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FVector SpawnLocation = GetActorLocation();
		// adjust, so dropped at player's feet
		SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

		// sanity check
		ensure(PickupClass);

		APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams);
		Pickup->InitializePickup(Item->GetClass(), DroppedQuantity);
	}
}


void AMain::UsePotion()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	if ((AvailableHealthPotions > 0) && (!bAttacking) && (!bIsDodging) && (!bEvading) && (MovementStatus != EMovementStatus::EMS_Staggered) && (!bInAir))
	{
		if (Health < MaxHealth)
		{
			bUsingPotion = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

			if (AnimInstance && UsePotionMontage) // if both are valid
			{
				AnimInstance->Montage_Play(UsePotionMontage, 1.0f);
				AnimInstance->Montage_JumpToSection(FName("UsePotion"));
			}
		}

		else if (NoNeedToUsePotionSound)  // already at full health
		{ UGameplayStatics::PlaySound2D(this, NoNeedToUsePotionSound, NoNeedToUsePotionSoundVolumeMultiplier); }

		ShowHUDCall();
	}
}


// triggered by animation (just after getting potion to mouth), in main anim blueprint
void AMain::ProcessHealthPotionEffect()
{
	AvailableHealthPotions -= 1;

	if (PotionHealSound)
	{ UGameplayStatics::PlaySound2D(this, PotionHealSound); }

	if (Health + HealthPotionRestoreAmount > MaxHealth)
	{ Health = MaxHealth; } // never exceed max health value

	else
	{ Health = Health + HealthPotionRestoreAmount; }

	DelayedHealthReportingValue = Health;
}


void AMain::UsePotionEnd()
{
	bUsingPotion = false;
}


// debug draw all pickups
void AMain::ShowPickupLocations()
{
	for (int32 i = 0; i < PickupLocations.Num(); i++)
	{ UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 25.f, 8, FLinearColor::Green, 10.f, 0.5f); }
}


/**
 *   combat
 */


bool AMain::Alive()
{
	return (GetMovementStatus() != EMovementStatus::EMS_Dead);
}


void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}


// for preventing double hits from a single enemy attack
void AMain::SetCanTakeDamage()
{
	if (GetMovementStatus() != EMovementStatus::EMS_Dead)
	{
		bCanTakeDamage = true;
		SetMovementStatus(EMovementStatus::EMS_Normal);
	}
}


void AMain::ResumeBlockAfterFailure()
{
	// ensure blocking stance resumes after hit react anim has played, if still holding block input down
	if (bBlockDown)
	{
		BlockUp();
		BlockDown();
	}
}


// condition check for attacking
bool AMain::CanAttack()
{
	return !bAttacking && !bUsingPotion && !bEvading && !bPerformingExecution && Stamina > 5 && !bIsMoveInputIgnored && !bIsEquipping && !bInAir
			&& MovementStatus != EMovementStatus::EMS_Staggered && MovementStatus != EMovementStatus::EMS_Dead;
}


// condition check for blocking
bool AMain::CanBlock()
{
	return !bUsingPotion && !bPerformingExecution && Stamina > 5 && !bIsMoveInputIgnored && !bIsEquipping && !bInAir
		&& MovementStatus != EMovementStatus::EMS_Staggered && MovementStatus != EMovementStatus::EMS_Dead;
}


void AMain::UnarmedAttackEnd()
{
	bAttacking = false;
	bHeavyAttacking = false;
	bCanJump = true;
	SetInterpToEnemy(false);
	EnableAttackRootMotionBP();
	bCanMove = true;

	if (bBlocking)
	{
		// ensure blocking stance resumes after anim has played
		BlockUp();
		BlockDown();
	}

	else
	{ bBlocking = false;}
}


void AMain::OnUnarmedAttackAnimationEnd()
{
	bPlayingUnarmedAttackAnim = false;
	bPlayingUnarmedRunningLightAttackAnim = false;
	bPlayingUnarmedRunningHeavyAttackAnim = false;
}


void AMain::DebugDeath()
{
	if (AMainPlayerController* PlayerController = Cast<AMainPlayerController>(GetController()))
	{
		PlayerController->ShowDeathScreen();
		Die();
	}
}


float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (!bCheatsOn && bCanTakeDamage && !bPerformingExecution && !bActiveIFrames)
	{
		// in case player is currently attacking and has been interrupted by taking damage, ensure we still call AttackEnd() (otherwise triggered by attack anim notify event)
		AttackEnd();
		if (EquippedWeapon)
		{
			// not set by AttackEnd (only at very end of animation via notify event), but can prevent dodging; so, clear
			EquippedWeapon->bPlayingAttackAnim = false;
			EquippedWeapon->bPlayingLightAttackAnim = false;	
			EquippedWeapon->bPlayingRunningLightAttackAnim = false;
			EquippedWeapon->bPlayingHeavyAttackAnim = false;
			EquippedWeapon->bPlayingRunningHeavyAttackAnim = false;

			// also, clear/reset the store weapon delay every time player takes damage (also reset when player attacks)
			GetWorldTimerManager().ClearTimer(EquippedWeapon->StoreWeaponOutOfCombatTimer);
			GetWorldTimerManager().SetTimer(EquippedWeapon->StoreWeaponOutOfCombatTimer, EquippedWeapon, &AWeapon::StoreWeapon, EquippedWeapon->StoreWeaponOutOfCombatDelayAmount);
		}
		
		AEnemy* Attacker = Cast<AEnemy>(DamageCauser);

		// if player is currently blocking with a shield
		if (bBlocking)
		{
			// check angle in order to determine if block is valid
			FRotator PlayerRot = GetActorRotation();
			FRotator DamageCauserRot = DamageCauser->GetActorRotation();
			float DeltaRotYaw = PlayerRot.Yaw - DamageCauserRot.Yaw;
			bool bFacingAttacker = false;

			// check angle for validity
			bBlockAttemptSucceeded = CheckBlockValidityBP(DamageCauser);
	
			// however, if enemy is doing a knockdown-capable attack, disregard block
			if (Attacker) // i.e., cast succeeded - not an arrow (only other damage causer besides enemies themselves)
			{
				if (Attacker->bKnockdownAttacking || Attacker->bPushbackAttacking || Attacker->bHeavyPushbackAttacking)
				{
					bBlockAttemptSucceeded = false;
					BlockUp();
				}
			}

			if (bBlockAttemptSucceeded)
			{
				bFacingAttacker = true;
				GetWorldTimerManager().SetTimer(BlockSuccessTrackingResetTimer, this, &AMain::ResetBlockSuccessTracking, 2.0f);
			}

			if (bFacingAttacker)
			{
				// deduct damage from stamina instead of health
				// reduce the overall stamina damage taken by a percent based on shield's stability rating
				float ShieldStabilityOffset = EquippedShield->ShieldConfig.Stability / 100;
				float StaminaDeductionAmount = DamageAmount - (DamageAmount * ShieldStabilityOffset);
				Stamina = Stamina -= StaminaDeductionAmount;

				// briefly delay stamina recovery
				bCanRegenStamina = false;
				GetWorldTimerManager().SetTimer(StaminaRegenDelayTimer, this, &AMain::ResetCanRegenStamina, BlockingStaminaRegenDelayAmount);

				// to prevent multiple instances of stamina damage stacking up too quickly (i.e., from the same attack) while blocking (uses same logic as taking health damage)
				bCanTakeDamage = false;
				GetWorldTimerManager().SetTimer(TakeDamageTimer, this, &AMain::SetCanTakeDamage, TakeDamageDelay);

				// if enough stamina damage to break guard
				if (Stamina <= 0.0f)
				{
					// play guard break animation, including halting input (movement/attack/etc) during
					UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

					if (AnimInstance && GuardBrokenAnim)
					{ AnimInstance->Montage_Play(GuardBrokenAnim, 0.8f); }

					// play shield's guard broken sound FX (instead of regular impact sound - not necessarily different)
					if (EquippedShield->ShieldGuardBrokenSound)
					{ UGameplayStatics::PlaySoundAtLocation(this, EquippedShield->ShieldGuardBrokenSound, GetActorLocation(), 1.0f, 1.0f, 0.0f); }

					// play the player's guard broken sound indicator FX
					if (PlayerGuardBrokenIndicatorSound)
					{ UGameplayStatics::PlaySoundAtLocation(this, PlayerGuardBrokenIndicatorSound, GetActorLocation(), 1.0f, 1.0f, 0.0f); }
				}

				else
				{
					// play regular blocking impact animation / FX
					UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

					if (AnimInstance && BlockingImpactWhileMovingAnim && BlockingImpactWhileIdleAnim)
					{
						UAnimMontage* AnimToPlay = BlockingImpactWhileMovingAnim;

						if (bIsIdle)
						{ AnimToPlay = BlockingImpactWhileIdleAnim; }

						AnimInstance->Montage_Play(AnimToPlay, 0.5f);

						// ensure blocking stance resumes after anim has played
						BlockUp();
						BlockDown();
					}

					// play shield hit sound FX
					if (EquippedShield->ShieldImpactSound)
					{ UGameplayStatics::PlaySoundAtLocation(this, EquippedShield->ShieldImpactSound, GetActorLocation(), 0.75f, 1.0f, 0.0f); }

					// play shield hit VFX
					if (BlockedHitParticles)
					{
						const USkeletalMeshSocket* ImpactFX_Socket = GetMesh()->GetSocketByName("WEAPON_L_ImpactFX_Socket");
						if (ImpactFX_Socket)
						{
							FVector SocketLocation = ImpactFX_Socket->GetSocketLocation(GetMesh());
							UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BlockedHitParticles, SocketLocation, FRotator(0.f), false);
						}
					}

					// play camera shake
					if (BlockingImpactCameraShake)
					{ GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(BlockingImpactCameraShake, 1.0f); }
				}

				return 0.0f;
			}

			// blocking, but wasn't valid - will take damage; if still holding block input when done playing hit react, resume blocking stance automatically
			if (Attacker)
			{
				if (!Attacker->bKnockdownAttacking)
				{ GetWorldTimerManager().SetTimer(ResumeBlockAfterFailureTimer, this, &AMain::ResumeBlockAfterFailure, 0.75f); }
			}

			else
			{ GetWorldTimerManager().SetTimer(ResumeBlockAfterFailureTimer, this, &AMain::ResumeBlockAfterFailure, 0.75f); }
		}

		// take health damage
		// reduce damage taken depending on physical damage reduction multiplier on equipped gear
		float ArmorDefenseOffset = CalculateArmorDefenseOffset();
		float HealthDeductionAmount = DamageAmount - (DamageAmount * ArmorDefenseOffset);

		Super::TakeDamage(HealthDeductionAmount, DamageEvent, EventInstigator, DamageCauser);
		const float DamageDealt = ModifyHealth(-HealthDeductionAmount);

		// also deduct same from stamina if knockdown attack (regardless of block attempt)
		if (Attacker)
		{
			if (Attacker->bKnockdownAttacking)
			{ Stamina = Stamina -= DamageAmount; }

			// interp player backwards slightly in addition to hit fx
			if (Attacker->bPushbackAttacking)
			{ PushBackPlayerBP(Attacker); }
		}

		// play hit sound fx
		if (HitSound1 && HitSound2 && HitSound3)
		{
				switch (HitSoundToPlay)
				{
				case 0:
					UGameplayStatics::PlaySoundAtLocation(this, HitSound1, GetActorLocation(), 1.0f, 1.0f, 0.0f);
					HitSoundToPlay += 1;
					break;

				case 1:
					UGameplayStatics::PlaySoundAtLocation(this, HitSound2, GetActorLocation(), 1.0f, 1.0f, 0.0f);
					HitSoundToPlay += 1;
					break;

				case 2:
					UGameplayStatics::PlaySoundAtLocation(this, HitSound3, GetActorLocation(), 1.0f, 1.0f, 0.0f);
					HitSoundToPlay = 0;
					break;

				default:
					;
				}
		}

		// play camera shake
		if (BlockingImpactCameraShake)
		{ GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(BlockingImpactCameraShake, 1.0f); }

		// if enough damage to be killed
		if (Health <= 0.0f)
		{
			if (AEnemy* KillerEnemy = Cast<AEnemy>(DamageCauser->GetOwner()))
			{ KilledByEnemy(DamageEvent, KillerEnemy, DamageCauser); }

			else // environmental kill, etc
			{ Killed(DamageEvent, DamageCauser); }
		}

		// flag player unable to take damage, and set timer to reallow it
		bCanTakeDamage = false;

		if (Attacker)
		{
			if (!Attacker->bKnockdownAttacking) // if player was knocked down, BP-side logic handles an extended delay window during getting up animation
			{ GetWorldTimerManager().SetTimer(TakeDamageTimer, this, &AMain::SetCanTakeDamage, TakeDamageDelay);}
		}

		else
		{ GetWorldTimerManager().SetTimer(TakeDamageTimer, this, &AMain::SetCanTakeDamage, TakeDamageDelay); }


		return DamageDealt;
	}

	return 0.0f;
}


// for non-enemy health reduction (i.e., environmental hazards, etc)
void AMain::DecrementHealth(float Amount)
{
	if (!bCheatsOn)
	{ Health = Health -= Amount; }
}


float AMain::ModifyHealth(const float Delta)
{
	const float OldHealth = Health;
	Health = FMath::Clamp<float>(Health + Delta, 0.0f, MaxHealth);

	return Health - OldHealth;
}


void AMain::OnRep_Health(float OldHealth)
{
	OnHealthModified(Health - OldHealth);
}


void AMain::Killed(struct FDamageEvent const& DamageEvent, const AActor* DamageCauser)
{
	if (AMainPlayerController* PlayerController = Cast<AMainPlayerController>(GetController()))
	{
		PlayerController->ShowDeathScreen();
		Die();

		GetWorldTimerManager().SetTimer(RespawnTimer, PlayerController, &AMainPlayerController::Respawn, RespawnDelay);
	}
}


void AMain::KilledByEnemy(struct FDamageEvent const& DamageEvent, class AEnemy* Enemy, const AActor* DamageCauser)
{
	Killed(DamageEvent, DamageCauser);
}


void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && DeathMontage) // if both are valid
	{
		AnimInstance->Montage_Play(DeathMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death1"));
	}

	SetupRagdollBP();
	ApplyDeathblowImpulseBP();

	SetMovementStatus(EMovementStatus::EMS_Dead);
	bCanJump = false;
}


void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	// turn off all collision volumes
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AMain::StartAttack()
{
	if (EquippedWeapon)
	{
		if (bHasWeaponDrawn)
		{
			bCanEvade = false;
			bCanMove = false;
			SetInterpToEnemy(true);
		}

		EquippedWeapon->StartAttack();
	}

	else
	{
		// if shield equipped but no weapon, auto unequip shield and proceed with unarmed attack logic
		if (EquippedShield) 
		{ 
			FindAndUnequipShieldBP();
			return;
		}

		bCanEvade = false;
		bCanMove = false;
		SetInterpToEnemy(true);
		BeginUnarmedAttack();
	}
}


void AMain::StopAttack()
{
	if (EquippedWeapon)
	{ EquippedWeapon->StopAttack(); }
}


void AMain::BeginUnarmedAttack()
{
	if (CanAttack())
	{
		bAttacking = true;

		if (!bCheatsOn)
		{
			// deduct stamina cost
			if (!bHeavyAttacking)
			{ Stamina -= UnarmedLightAttackStaminaCost; }

			else
			{ Stamina -= UnarmedHeavyAttackStaminaCost; }

			// briefly delay stamina recovery
			bCanRegenStamina = false;
			GetWorldTimerManager().SetTimer(StaminaRegenDelayTimer, this, &AMain::ResetCanRegenStamina, AttackingStaminaRegenDelayAmount);
		}

		// set to a default
		UAnimMontage* AnimToPlay = UnarmedLightAttack1Montage;

		// light attack
		if (!bHeavyAttacking)
		{
			if (MovementStatus == EMovementStatus::EMS_Sprinting)
			{
				AnimToPlay = UnarmedRunningLightAttackMontage;
				bPlayingUnarmedRunningLightAttackAnim = true;
			}

			else
			{
				if (bUnarmedLightAttackFollowUpWindowOpen)
				{ LightAttackAnimationCounter += 1; }

				else
				{ LightAttackAnimationCounter = 0; }

				switch (LightAttackAnimationCounter)
				{
				case 0:
					AnimToPlay = UnarmedLightAttack1Montage;
					break;

				case 1:
					AnimToPlay = UnarmedLightAttack2Montage;
					break;

				case 2:
					AnimToPlay = UnarmedLightAttack3Montage;
					LightAttackAnimationCounter = -1;
					break;

				default:
					;
				}

				bPlayingUnarmedAttackAnim = true;
			}

			// play animations
			PlayAnimMontage(AnimToPlay);
		}

		// heavy attack
		else
		{
			if (MovementStatus == EMovementStatus::EMS_Sprinting)
			{
				AnimToPlay = UnarmedRunningHeavyAttackMontage;
				bPlayingUnarmedRunningHeavyAttackAnim = true;
			}

			else
			{
				if (bUnarmedHeavyAttackFollowUpWindowOpen)
				{ HeavyAttackAnimationCounter += 1; }

				else
				{ HeavyAttackAnimationCounter = 0; }

				switch (HeavyAttackAnimationCounter)
				{
				case 0:

					AnimToPlay = UnarmedHeavyAttack1Montage;
					break;

				case 1:
					AnimToPlay = UnarmedHeavyAttack2Montage;
					HeavyAttackAnimationCounter = -1;
					break;

				default:
					;
				}

				bPlayingUnarmedAttackAnim = true;
			}

			// play animations
			PlayAnimMontage(AnimToPlay);
		}
	}

	else
	{
		// if heavy attack input is rejected due to insufficient stamina, still clear flag so next valid attack isn't also a heavy if input is actually for light attack
		bHeavyAttacking = false;
	}
}


void AMain::StartBlock()
{
	if (EquippedShield)
	{
		if (EquippedWeapon)
		{
			if (!bHasWeaponDrawn) // need shield/weapon out to block
			{ EquippedWeapon->DrawWeapon(); }
		}

		EquippedShield->StartBlock();
	}
}


void AMain::StopBlock()
{
	if (EquippedShield)
	{
		EquippedShield->StopBlock();
		CurrentStaminaRegenRate = DefaultStaminaRegenRate; // reset stamina regen rate
	}
}


void AMain::AttackEnd()
{
	bAttacking = false;
	bHeavyAttacking = false;
	bChargingHeavyAttack = false;
	bIsDodging = false;
	bIsRolling = false;
	bCanJump = true;
	SetInterpToEnemy(false);
	EnableAttackRootMotionBP();
	bCanMove = true;
}


void AMain::BlockEnd()
{
	bBlocking = false;
}


// for spawning currently stored weapon mesh onto back storage socket
void AMain::SpawnStoredWeapon(FVector Location, FRotator Rotation)
{
	FActorSpawnParameters SpawnParams;
	AActor* SpawnedActorRef = GetWorld()->SpawnActor<AActor>(StoredWeaponToSpawn, Location, Rotation, SpawnParams);

	// attach potion to left-hand socket
	const USkeletalMeshSocket* WeaponStorageSocket = GetMesh()->GetSocketByName("Weapon_Storage_Socket");
	
	if (WeaponStorageSocket)
	{ WeaponStorageSocket->AttachActor(SpawnedActorRef, GetMesh()); }
}

#undef LOCTEXT_NAMESPACE