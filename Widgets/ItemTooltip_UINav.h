// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "UINavWidget.h"
#include "ItemTooltip_UINav.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPGPROJECT_API UItemTooltip_UINav : public UUINavWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Tooltip", meta = (ExposeOnSpawn = true))
	class UInventoryItemWidget_UINav* InventoryItemWidget;
	
};
