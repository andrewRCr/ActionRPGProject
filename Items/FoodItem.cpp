// © 2022 Andrew Creekmore 


#include "../Items/FoodItem.h"

#define LOCTEXT_NAMESPACE "FoodItem"

UFoodItem::UFoodItem()
{
	HealAmount = 20.f;
	UseActionText = LOCTEXT("ItemUseActionText", "Consume");
}


void UFoodItem::Use(class AMain* Character)
{
	// heal character here
	UE_LOG(LogTemp, Warning, TEXT("Nom nom nom."));
}

#undef LOCTEXT_NAMESPACE