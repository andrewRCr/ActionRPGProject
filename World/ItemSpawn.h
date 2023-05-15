// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/TargetPoint.h"
#include "ItemSpawn.generated.h"

USTRUCT(BlueprintType)
struct FLootTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	// the item(s) to spawn
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TArray<TSubclassOf<class UItem>> Items;

	// the percentage chance of spawning this item if we hit it on the roll
	UPROPERTY(EditDefaultsOnly, Category = "Loot", meta = (ClampMin = 0.001, ClampMax = 1.0))
	float Probability = 1.0f;

};

/**
 * 
 */
UCLASS()
class ACTIONRPGPROJECT_API AItemSpawn : public ATargetPoint
{
	GENERATED_BODY()

public:
	
	// sets default values for this actor's properties
	AItemSpawn();

	UPROPERTY(EditAnywhere, Category = "Loot")
	class UDataTable* LootTable;

	// since pickups use a BP base, use a UPROPERTY to select 
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TSubclassOf<class APickup> PickupClass;


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AActor*> SpawnedPickups;

	virtual void BeginPlay() override;

	UFUNCTION()
	void SpawnItem();

	// bound to the item being destroyed; can queue up another to be spawned in (if desired)
	UFUNCTION()
	void OnItemTaken(AActor* DestroyedActor);
};
