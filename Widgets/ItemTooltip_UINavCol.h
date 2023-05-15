// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "UINavCollection.h"
#include "ItemTooltip_UINavCol.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPGPROJECT_API UItemTooltip_UINavCol : public UUINavCollection
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Tooltip", meta = (ExposeOnSpawn = true))
	class UInventoryItemWidget_UINavCol* InventoryItemWidget;
	
};
