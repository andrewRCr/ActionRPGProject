// © 2022 Andrew Creekmore 


#include "../World/ItemSpawn.h"
#include "../Items/Item.h"
#include "../World/Pickup.h"

AItemSpawn::AItemSpawn()
{
	PrimaryActorTick.bCanEverTick = false;
}


void AItemSpawn::BeginPlay()
{
	Super::BeginPlay();

	SpawnItem();
}


void AItemSpawn::SpawnItem()
{
	if (LootTable)
	{
		TArray<FLootTableRow*> SpawnItems;
		LootTable->GetAllRows("", SpawnItems);

		const FLootTableRow* LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];

		ensure(LootRow);

		float ProbabilityRoll = FMath::FRandRange(0.0f, 1.0f);

		while (ProbabilityRoll > LootRow->Probability)
		{
			LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
			ProbabilityRoll = FMath::FRandRange(0.0f, 1.0f);
		}

		// ensure hit valid row and that row has items to spawn
		if (LootRow && LootRow->Items.Num() && PickupClass)
		{
			float Angle = 0.0f;

			for (auto& ItemClass : LootRow->Items)
			{
				// ensure that if spawning multiple items, they spawn in a circle
				const FVector LocationOffset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * 50.0f;

				FActorSpawnParameters SpawnParams;
				SpawnParams.bNoFail = true;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				const int32 ItemQuantity = ItemClass->GetDefaultObject<UItem>()->GetQuantity();

				FTransform SpawnTransform = GetActorTransform();
				SpawnTransform.AddToTranslation(LocationOffset);

				// actually spawn pickup

				APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams);
				Pickup->InitializePickup(ItemClass, ItemQuantity);
				Pickup->OnDestroyed.AddUniqueDynamic(this, &AItemSpawn::OnItemTaken);

				// add to tracking array
				SpawnedPickups.Add(Pickup);

				// increment the angle for the location offset
				Angle += (PI * 2.0f) / LootRow->Items.Num();
			}
		}
	}
}


void AItemSpawn::OnItemTaken(AActor* DestroyedActor)
{
	SpawnedPickups.Remove(DestroyedActor);
}
