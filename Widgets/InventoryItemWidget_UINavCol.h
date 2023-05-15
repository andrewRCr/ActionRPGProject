// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "UINavCollection.h"
#include "InventoryItemWidget_UINavCol.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPGPROJECT_API UInventoryItemWidget_UINavCol : public UUINavCollection
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Widget", meta = (ExposeOnSpawn = true))
	class UItem* Item;
	
};
