// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "../World/EnemySpawn.h"
#include "EnemySpawnTriggerBox.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPGPROJECT_API AEnemySpawnTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

public:

	// all enemy spawns assigned to this trigger instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variable Settings")
	TArray<AEnemySpawn*> EnemySpawnsToTrigger;
	
};
