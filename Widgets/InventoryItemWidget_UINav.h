// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "UINavWidget.h"
#include "InventoryItemWidget_UINav.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPGPROJECT_API UInventoryItemWidget_UINav : public UUINavWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Item Widget", meta = (ExposeOnSpawn = true))
	class UItem* Item;
	
};
