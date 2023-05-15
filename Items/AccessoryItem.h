// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "../Items/EquippableItem.h"
#include "AccessoryItem.generated.h"

UENUM(BlueprintType)
enum class EAccessoryItemType : uint8
{
	EAI_HealingFlask UMETA(DisplayName = "HealingFlask"),

	EAI_MAX UMETA(DisplayName = "DefaultMAX")
};


/**
 * 
 */
UCLASS(Blueprintable)
class ACTIONRPGPROJECT_API UAccessoryItem : public UEquippableItem
{
	GENERATED_BODY()

public:

	UAccessoryItem();

	virtual bool Equip(class AMain* Character) override;
	virtual bool Unequip(class AMain* Character) override;



	// accessory data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories")
	EAccessoryItemType AccessoryItemType;
	
};
