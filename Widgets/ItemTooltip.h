// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTooltip.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPGPROJECT_API UItemTooltip : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Tooltip", meta = (ExposeOnSpawn = true))
	class UInventoryItemWidget* InventoryItemWidget;
	
};
