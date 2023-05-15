// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Items/EquippableItem.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "Main.generated.h"


USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	FInteractionData()
	{
		ViewedInteractionComponent = nullptr;
		LastInteractionCheckTime = 0.f;
		bInteractHeld = false;
	}

	// the current interactable component we're viewing, if there is one
	UPROPERTY()
	class UInteractionComponent* ViewedInteractionComponent;
	
	// the time when we last checked for an interactable
	UPROPERTY()
	float LastInteractionCheckTime;

	// whether the player is holding the interact key
	UPROPERTY()
	bool bInteractHeld;

};

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Crouched UMETA(DisplayName = "Crouched"),
	EMS_Walking UMETA(DisplayName = "Walking"),
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Idle UMETA(DisplayName "Idle"),
	EMS_Staggered UMETA(DisplayName "Staggered"),
	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "DefaultMax")

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquippableSlot, Slot, const UEquippableItem*, Item);


UCLASS()
class ACTIONRPGPROJECT_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	// map of meshes to have "equipped" if no items equipped - i.e., bare skin meshes
	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	TMap<EEquippableSlot, USkeletalMesh*> NakedMeshes;

	// "equipped" meshes
	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	TMap<EEquippableSlot, USkeletalMeshComponent*> MainMeshes;

	// main character's inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInventoryComponent* PlayerInventory;

	/**
	*  setup modular skeletal mesh
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* HairMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* HelmetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* LegsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* FeetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* CloakMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* Hand_R_Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* Hand_L_Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* Weapon_R_Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* Weapon_L_Mesh;

	/**
	*  setup HUD
	*/

	 // reference to the UMG asset in the editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	// variable to hold the widget after creation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

	/**
	*  setup camera
	*/

	/** camera boom positioning the camera behind the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom2;

	/** follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** base turn rates to scale turning functions for the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;
	
	// player controller
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;


	/**
	 *  player stats: movement and stamina defaults / costs
	 */

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	FORCEINLINE EMovementStatus GetMovementStatus() { return MovementStatus; }

	/** set movement status and running speed */
	void SetMovementStatus(EMovementStatus Status);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Speeds")
	float WalkingSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Speeds")
	float RunningSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Speeds")
	float SprintingSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Speeds")
	float CrouchedSpeed;

	float IdleSpeed;

	// distance covered by an evasion move
	float EvadeDistance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bCanRegenStamina;

	float RegenDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CurrentStaminaRegenRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DefaultStaminaRegenRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BlockingStaminaRegenRateModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BlockingStaminaRegenDelayAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AttackingStaminaRegenDelayAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ExhaustedStaminaRegenDelayAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DodgeStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RollStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaRegenDelayCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaRegenDelayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed")
	float UnarmedLightAttackStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed")
	float UnarmedHeavyAttackStaminaCost;

	/**
	*  movement modifiers
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsMovementLocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsIdle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsMoveInputIgnored;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bCanEvade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bCanMove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bInAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsWalking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsCrouching;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bCanJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsDodging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsRolling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsEquipping;

	FTimerHandle DodgeResetTimer;

	FTimerHandle StaminaRegenDelayTimer;

	FTimerHandle ResumeBlockAfterFailureTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed")
	bool bPlayingUnarmedAttackAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed")
	bool bPlayingUnarmedRunningLightAttackAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed")
	bool bPlayingUnarmedRunningHeavyAttackAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed")
	bool bUnarmedLightAttackFollowUpWindowOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed")
	bool bUnarmedHeavyAttackFollowUpWindowOpen;

	/**
	 *  player stats: health, respawn delay, and unarmed damage
	 */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float DelayedHealthReportingValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float DelayedHealthBarValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float DelayedHealthBarDrainRate;

	FTimerHandle RespawnTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float RespawnDelay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed")
	float UnarmedLightAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unarmed")
	float UnarmedHeavyAttackDamage;

	 /**
	  *   combat modifiers
	  */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	bool bCheatsOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bHeavyAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bChargingHeavyAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bShieldAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bLockedOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bCanTakeDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bBlocking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bBlockAttemptSucceeded;

	FTimerHandle BlockSuccessTrackingResetTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bPerformingExecution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FHitResult LastHitResult;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName LastHitBone;

	/**
	*   control modifiers
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShiftKeyDown;
	bool bInteractKeyDown;
	bool bLightAttackDown;
	bool bBlockDown;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bHeavyAttackDown;
	bool bDodgePressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEvading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActiveIFrames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsingPotion;

	/**
	*  animation modifiers 
	*/

	int32 LightAttackAnimationCounter;
	int32 HeavyAttackAnimationCounter;
	int32 LightForwardAnimationCounter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bShouldPlayWeightShiftSoundWhenBlockBegins;

	/**
	 *  animations & effects
	 */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* BeginPlayMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* PickupWaistLevelMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* AllDodgesMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* LockedOnRollMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* UnlockedRollMontage;

	// blocking impact animation for use when idle
	UPROPERTY(EditDefaultsOnly, Category = "Anims")
	UAnimMontage* BlockingImpactWhileIdleAnim;

	// blocking impact animation for use when moving (i.e., upper body only)
	UPROPERTY(EditDefaultsOnly, Category = "Anims")
	UAnimMontage* BlockingImpactWhileMovingAnim;

	// blocking guard broken animation (i.e., stamina fully depleted by blocked hit)
	UPROPERTY(EditDefaultsOnly, Category = "Anims")
	UAnimMontage* GuardBrokenAnim;

	// impact hit particle system (when block successful)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	class UParticleSystem* BlockedHitParticles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* HurtMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* UsePotionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TSubclassOf<UCameraShakeBase> BlockingImpactCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TSubclassOf<UCameraShakeBase> HitImpactCameraShake;

	/**
	 *  sounds
	 */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* HitSound1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* HitSound2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* HitSound3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* DodgeSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* PotionHealSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* PlayerGuardBrokenIndicatorSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	bool bShouldPlayGearEquipUnequipSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* NoNeedToUsePotionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	float NoNeedToUsePotionSoundVolumeMultiplier;

	/**
	 *  items / weapons / inventory
	 */

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<AActor> PotionToSpawn;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<AActor> StoredWeaponToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	int32 Coins;

	TArray<FVector> PickupLocations;

	UPROPERTY(VisibleAnywhere)
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere)
	class AShield* EquippedShield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	float ShieldSpawnDelay;

	FTimerHandle TimerHandle_SpawnShield;

	class UShieldItem* EquippedShieldItem;

	// equip (not draw) animation, if any
	UPROPERTY(EditDefaultsOnly, Category = "Items")
	UAnimMontage* ShieldEquipAnim;

	// unequip (not store) animation, if any
	UPROPERTY(EditDefaultsOnly, Category = "Items")
	UAnimMontage* ShieldUnequipAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasWeaponEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasWeaponDrawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	int32 TotalHealthPotionCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	int32 AvailableHealthPotions;

	float HealthPotionRestoreAmount;

	/**
	 *   combat
	 */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bInSoftLockRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float RInterpToEnemySpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float VInterpToEnemySpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float VInterpToEnemySpeedUnarmed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float TooFarToVInterpThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float TooCloseToVInterpThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float TooCloseToVInterpThresholdUnarmed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float TooCloseBackupVInterpSpeed;

	bool bInterpToEnemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class AEnemy* CombatTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class AActor* LastTargetedEnemy;

	int32 HitSoundToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AEnemy> EnemyFilter;

	FTimerHandle TakeDamageTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float TakeDamageDelay;

	/**
	 *   interaction modifiers and data
	 */

	// how often in escond to check for an interactable object; set to zero for every tick
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckFrequency;

	// how far we'll trace when we check if the player is looking at an interactable object
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckDistance;

	// information about the current state of the player's interaction
	UPROPERTY()
	FInteractionData InteractionData;

	FTimerHandle TimerHandle_Interact;

protected:

	// called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Restart() override;

	UPROPERTY(BlueprintReadOnly)
	UInventoryComponent* LootSource;

	UFUNCTION()
	void OnLootSourceOwnerDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnRep_LootSource();

	// allows for efficient access of equipped items
	UPROPERTY(VisibleAnywhere, Category = "Items")
	TMap<EEquippableSlot, UEquippableItem*> EquippedItems;

public:	

	// called every frame
	virtual void Tick(float DeltaTime) override;

	// getters/setters for Camera Boom and Follow Camera
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/**
	 *   interactions
	 */

	void PerformInteractionCheck();

	void CouldntFindInteractable();
	void FoundNewInteractable(UInteractionComponent* Interactable);

	UPROPERTY(BlueprintReadOnly)
	bool bInteractableFoundOnLastCheck;

	UFUNCTION(BlueprintCallable)
	void BeginInteract();

	UFUNCTION(BlueprintCallable)
	void EndInteract();

	void Interact();

	// returns true if we're interacting with an item that has an interaction time (e.g., a fireplace that takes 2 seconds to light)
	bool IsInteracting() const;

	// gets the time left until we complete/execute interacting with the current interactable
	float GetRemainingInteractTime();

	// helper function to make grabbing interactable faster
	FORCEINLINE class UInteractionComponent* GetInteractable() const { return InteractionData.ViewedInteractionComponent; }

	/**
	 *   input
	 */

	// called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** called via input to turn at a given rate
	* @param Rate -- this is a normalized rate, i.e., 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/** called via input to look up/down at a given rate
	* @param Rate -- this is a normalized rate, i.e., 1.0 means 100% of desired look up/down rate
	*/
	void LookUpAtRate(float Rate);

	void JumpStart();

	// pressed down to enable sprinting
	void ShiftKeyDown();

	// released to stop sprinting
	void ShiftKeyUp();

	// pressed down to interact
	UFUNCTION(BlueprintCallable)
	void InteractKeyDown();
	UFUNCTION(BlueprintCallable)
	void InteractKeyUp();

	// pressed down to heavy attack
	void HeavyAttackDown();
	void HeavyAttackUp();

	// pressed down to light attack
	void LightAttackDown();
	void LightAttackUp();

	UFUNCTION(BlueprintCallable)
	void BlockDown();
	UFUNCTION(BlueprintCallable)
	void BlockUp();

	// pressed down to enable slow walking 
	void WalkStart();

	// released to stop slow walking
	void WalkStop();

	// pressed down to toggle slow walking
	void WalkToggle();

	// pressed down to enable crouching
	void CrouchStart();

	// released to stop crouching
	void CrouchStop();

	// pressed down to toggle crouch on/off
	void CrouchToggle();

	void ResetCanRegenStamina();

	void PlayDodgeSound();

	FVector GetMovementDirection();

	void DeveloperCheatsToggle();

	/**
	 *   items / inventory
	 */

	UFUNCTION(BlueprintCallable)
	void SetLootSource(class UInventoryComponent* NewLootSource);

	UFUNCTION(BlueprintPure, Category = "Looting")
	bool IsLooting() const;

	UFUNCTION(BlueprintCallable, Category = "Looting")
	void LootItem(class UItem* ItemToGive);

	// handle equipping an equippable item
	bool EquipItem(class UEquippableItem* Item);
	bool UnequipItem(class UEquippableItem* Item);

	// should never be called directly; UGearItem, UAccessoryItem and UWeaponItem call these on top of EquipItem
	void EquipGear(class UGearItem* Gear);
	void UnequipGear(const EEquippableSlot Slot, class UGearItem* Gear);

	void EquipWeapon(class UWeaponItem* WeaponItem);
	void UnequipWeapon();

	void EquipShield(class UShieldItem* ShieldItem);
	void SpawnShieldAndFinishEquip();

	void UnequipShield();
	void DespawnShieldAndFinishUnequip();

	void EquipAccessory(class UAccessoryItem* AccessoryItem);
	void UnequipAccessory(class UAccessoryItem* AccessoryItem);

	// called to update the inventory
	UPROPERTY(BlueprintAssignable, Category = "Items")
	FOnEquippedItemsChanged OnEquippedItemsChanged;

	// given a slot, returns skeletal mesh component
	UFUNCTION(BlueprintPure)
	class USkeletalMeshComponent* GetSlotSkeletalMeshComponent(const EEquippableSlot Slot);

	// helper function; expose otherwise protected EquippedItems
	UFUNCTION(BlueprintPure)
	FORCEINLINE TMap<EEquippableSlot, UEquippableItem*> GetEquippedItems() const { return EquippedItems; }

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	FORCEINLINE class AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	FORCEINLINE class AShield* GetEquippedShield() const { return EquippedShield; }

	// use an item from our inventory
	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class UItem* Item);

	// drop item from inventory
	UFUNCTION(BlueprintCallable, Category = "Items")
	void DropItem(class UItem* Item, const int32 Quantity);

	// needed as pickups use a BP base class
	UPROPERTY(EditDefaultsOnly, Category = "Items")
	TSubclassOf<class APickup> PickupClass;

	UFUNCTION(BlueprintCallable)
	void SpawnStoredWeapon(FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintCallable)
	void ShowPickupLocations();

	void UsePotion();

	UFUNCTION(BlueprintCallable)
	void ProcessHealthPotionEffect();

	UFUNCTION(BlueprintCallable)
	void UsePotionEnd();

	/**
	 *   combat
	 */

	UFUNCTION(BlueprintCallable)
	void SetInterpToEnemy(bool Interp);

	UFUNCTION(BlueprintCallable)
	FRotator GetLookAtRotationYaw(FVector Target);

	void ResetBlockSuccessTracking();

	// use inherited TakeDamage function (built-in)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// non-enemy damage (instead of using TakeDamage; may not need at all later)
	void DecrementHealth(float Amount);

	// modify player health by either a positive or negative amount; return amount of health actually removed
	float ModifyHealth(const float Delta);

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION(BlueprintImplementableEvent)
	void OnHealthModified(const float HealthDelta);

	// called when killed (by suicide or something else, e.g. environment)
	void Killed(struct FDamageEvent const& DamageEvent, const AActor* DamageCauser);
	void KilledByEnemy(struct FDamageEvent const& DamageEvent, class AEnemy* Enemy, const AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeath();

	void Die();

	// setter for combat target
	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	// setter for bHasCombatTarget
	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }

	UFUNCTION(BlueprintCallable)
	void StartAttack();

	void StopAttack();

	void BeginUnarmedAttack();

	void StartBlock();

	void StopBlock();

	UPROPERTY()
	float LastUnarmedAttackTime;

	UPROPERTY(EditDefaultsOnly, Category = "Unarmed")
	float UnarmedAttackDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Unarmed")
	float UnarmedAttackDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Unarmed")
	class UAnimMontage* UnarmedLightAttack1Montage;

	UPROPERTY(EditDefaultsOnly, Category = "Unarmed")
	class UAnimMontage* UnarmedLightAttack2Montage;

	UPROPERTY(EditDefaultsOnly, Category = "Unarmed")
	class UAnimMontage* UnarmedLightAttack3Montage;

	UPROPERTY(EditDefaultsOnly, Category = "Unarmed")
	class UAnimMontage* UnarmedRunningLightAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Unarmed")
	class UAnimMontage* UnarmedHeavyAttack1Montage;

	UPROPERTY(EditDefaultsOnly, Category = "Unarmed")
	class UAnimMontage* UnarmedHeavyAttack2Montage;

	UPROPERTY(EditDefaultsOnly, Category = "Unarmed")
	class UAnimMontage* UnarmedRunningHeavyAttackMontage;

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void BlockEnd();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	UFUNCTION(BlueprintCallable)
	bool Alive();

	UFUNCTION(BlueprintCallable)
	void SetCanTakeDamage();

	UFUNCTION(BlueprintImplementableEvent)
	bool CheckBlockValidityBP(AActor* DamageCauser);

	void ResumeBlockAfterFailure();

	UFUNCTION(BlueprintImplementableEvent)
	void ApplyDeathblowImpulseBP();

	UFUNCTION(BlueprintImplementableEvent)
	void SetupRagdollBP();

	UFUNCTION(BlueprintCallable)
	bool CanAttack();

	UFUNCTION(BlueprintCallable)
	bool CanBlock();

	UFUNCTION(BlueprintCallable)
	void UnarmedAttackEnd();

	UFUNCTION(BlueprintCallable)
	void OnUnarmedAttackAnimationEnd();

	UFUNCTION(BlueprintImplementableEvent)
	void FindAndUnequipShieldBP();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayBlockWeightShiftSFX();

	UFUNCTION(BlueprintImplementableEvent)
	void ComparePotentialSoftLockTargets();

	UFUNCTION(BlueprintImplementableEvent)
	void DisableAttackRootMotionBP();

	UFUNCTION(BlueprintImplementableEvent)
	void EnableAttackRootMotionBP();

	UFUNCTION(BlueprintImplementableEvent)
	void PushBackPlayerBP(AEnemy* Attacker);

	UFUNCTION(BlueprintImplementableEvent)
	float CalculateArmorDefenseOffset();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayGearEquipSFX(bool bEquipping, USoundCue* Sound);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHUDCall();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHUDCall(); 

	/**
	* @param isTearingDown true if world is tearing down.
	*/UFUNCTION(BluePrintCallable, BlueprintPure, Category = "WorldState", meta = (DisplayName = "IsTearingDown", DefaultToSelf = caller, HidePin = caller))
	static void K2_IsTearingDown(UObject* caller, bool& isTearingDown)
	{
		isTearingDown = caller->GetWorld()->bIsTearingDown;
	}


	UFUNCTION(BlueprintCallable)
	void DebugDeath();
};