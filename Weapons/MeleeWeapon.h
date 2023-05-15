// // © 2022 Andrew Creekmore 

#pragma once

#include "../Enemies/Enemy.h"
#include "../Character/Main.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "CoreMinimal.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

#include "MeleeWeapon.generated.h"


UENUM(BlueprintType)
enum class EMeleeWeaponState : uint8
{
	EWS_Pickup UMETA(DisplayName = "Pickup"),
	EWS_EquippedRight UMETA(DisplayName = "EquippedRight"),
	EWS_EquippedLeft UMETA(DisplayName = "EquippedLeft"),

	EWS_MAX UMETA(DisplayName = "DefaultMax")
};


UENUM(BlueprintType)
enum class EMeleeWeaponType : uint8
{
	EWS_StraightSword UMETA(DisplayName = "StraightSword"),

	EWS_MAX UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class ACTIONRPGPROJECT_API AMeleeWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeleeWeapon();

	// MESH, COLLISION, INTERACTION PROMPT

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Combat")
	class UBoxComponent* CombatCollision;

	UPROPERTY(EditAnywhere)
	UWidgetComponent* InteractionWidget;

	// SOUNDS

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon | Sound")
	class USoundCue* OnEquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon | Sound")
	USoundCue* LightSwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon | Sound")
	USoundCue* HeavySwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon | Sound")
	USoundCue* WeaponHitEnemySound1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon | Sound")
	USoundCue* WeaponHitEnemySound2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon | Sound")
	USoundCue* WeaponHitEnemySound3;

	int32 WeaponHitEnemySoundCounter;

	// STATS & MODIFIERS

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Melee Weapon")
	EMeleeWeaponState MeleeWeaponState;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Melee Weapon")
	EMeleeWeaponType MeleeWeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon")
	bool bRightHandEquippable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon | Combat")
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon | Combat")
	float BaseHeavyDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapon | Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	// WEAPON USER / PLAYER CONTROLLER
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee Weapon | Combat")
	class AMain* CurrentlyEquippedBy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Melee Weapon | Combat")
	AController* WeaponInstigator;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// STATE, EQUIPPING AND STORING

	FORCEINLINE void SetWeaponState(EMeleeWeaponState State) { MeleeWeaponState = State; }
	FORCEINLINE EMeleeWeaponState GetWeaponState() { return MeleeWeaponState; }

	FORCEINLINE void SetInstigator(AController* Inst) { WeaponInstigator = Inst; }

	UFUNCTION(BlueprintCallable)
	void EquipRight(class AMain* Char);

	UFUNCTION(BlueprintCallable)
	void StoreRight(class AMain* Char);

	// COLLISION OVERLAP / ACTIVATION

	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void ActivateCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();



};
