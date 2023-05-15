// © 2022 Andrew Creekmore 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.h"
#include "NavigationSystem.h"
#include "Wendigo.generated.h"



UCLASS()
class ACTIONRPGPROJECT_API AWendigo : public AEnemy
{
	GENERATED_BODY()

public:
	
	// sets default values for this character's properties
	AWendigo();

	class UNavigationSystemV1* NavSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsAttackingSwipe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsAttackingHeadButt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsAttackingBite;

	FTimerHandle WendigoAttackTimer;

	FTimerHandle WendigoResetAttackCounterTimer;

	FTimerHandle MoveToTargetTimer;

	int32 WendigoAttackCounter;

protected:
	
	// called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual void AttackEnd() override;

	void ResetAttackCounter();
};
