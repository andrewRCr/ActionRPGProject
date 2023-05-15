// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "../Items/Item.h"
#include "FoodItem.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPGPROJECT_API UFoodItem : public UItem
{
	GENERATED_BODY()

public:
	
	UFoodItem();

	// amount of health the food item heals the player
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Healing")
	float HealAmount;

	virtual void Use(class AMain* Character) override;
	
};
