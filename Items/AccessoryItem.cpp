// © 2022 Andrew Creekmore 


#include "../Items/AccessoryItem.h"
#include "../DebugMacros.h"
#include "../Character/Main.h"

UAccessoryItem::UAccessoryItem()
{
	AccessoryItemType = EAccessoryItemType::EAI_MAX;
}

bool UAccessoryItem::Equip(class AMain* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character)
	{ Character->EquipAccessory(this); }

	return bEquipSuccessful;
}

bool UAccessoryItem::Unequip(class AMain* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character)
	{ Character->EquipAccessory(this); }

	return bEquipSuccessful;
}

