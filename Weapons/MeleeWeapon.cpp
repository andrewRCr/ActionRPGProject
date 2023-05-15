// // © 2022 Andrew Creekmore 


#include "MeleeWeapon.h"

// sets default values
AMeleeWeapon::AMeleeWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetRootComponent());

	// default is right hand
	bRightHandEquippable = true;

	// default state is in world, as an item pickup
	MeleeWeaponState = EMeleeWeaponState::EWS_Pickup;

	// default type unset (set on children)
	// default base damages unset (set on children)

	//BaseDamage = 25.f;
	//BaseHeavyDamage = 50.f;

	CurrentlyEquippedBy = NULL;

	WeaponHitEnemySoundCounter = 0;

}

// Called when the game starts or when spawned
void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeapon::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AMeleeWeapon::CombatOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

// Called every frame
void AMeleeWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AMeleeWeapon::EquipRight(AMain* Char)
{
	if (Char) // if character is valid
	{
		SetInstigator(Char->GetController());

		// keep camera from zooming-in if weapon is in the way / no pawn collision
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		SkeletalMesh->SetSimulatePhysics(false);

		// make no longer interactive (don't appear in checks for nearby interactive objects)
		SkeletalMesh->SetCollisionObjectType(ECC_WorldDynamic);

		// attach weapon to right-hand socket
		const USkeletalMeshSocket* RightHandSocket = Char->GetMesh()->GetSocketByName("WEAPON_R");
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(this, Char->GetMesh());

			//Char->GetEquippedRightWeapon()->Destroy();
			//Char->SetEquippedRightWeapon(this);
			CurrentlyEquippedBy = Char;

			// for interact prompt widget removal
			MeleeWeaponState = EMeleeWeaponState::EWS_EquippedRight;



		}

		if (OnEquipSound)
		{
			UGameplayStatics::PlaySound2D(this, OnEquipSound);
		}

	}
}


void AMeleeWeapon::StoreRight(AMain* Char)
{
	if (Char) // if character is valid
	{
		SetInstigator(Char->GetController());

		// keep camera from zooming-in if weapon is in the way / no pawn collision
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		SkeletalMesh->SetSimulatePhysics(false);

		// make no longer interactive (don't appear in checks for nearby interactive objects)
		SkeletalMesh->SetCollisionObjectType(ECC_WorldDynamic);

		// attach weapon to Weapon_Storage_Socket socket
		const USkeletalMeshSocket* WeaponStorageSocket = Char->GetMesh()->GetSocketByName("Weapon_Storage_Socket");
		if (WeaponStorageSocket)
		{
			WeaponStorageSocket->AttachActor(this, Char->GetMesh());
			//bRotate = false;

			//Char->GetEquippedRightWeapon()->Destroy();
			//Char->SetEquippedRightWeapon(this);
			CurrentlyEquippedBy = Char;

			// for interact prompt widget removal
			MeleeWeaponState = EMeleeWeaponState::EWS_EquippedRight;

		}
	}
}



void AMeleeWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (Enemy->HitParticles)
			{
				const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
				if (WeaponSocket)
				{
					FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}

			if (WeaponHitEnemySound1 && WeaponHitEnemySound2 && WeaponHitEnemySound3)
			{
				switch (WeaponHitEnemySoundCounter)

				{
				case 0:
					UGameplayStatics::PlaySound2D(this, WeaponHitEnemySound1);
					WeaponHitEnemySoundCounter += 1;
					break;

				case 1:
					UGameplayStatics::PlaySound2D(this, WeaponHitEnemySound2);
					WeaponHitEnemySoundCounter += 1;
					break;

				case 2:
					UGameplayStatics::PlaySound2D(this, WeaponHitEnemySound3);
					WeaponHitEnemySoundCounter = 0;
					break;

				default:
					;
				}
			}

			if (DamageTypeClass)
			{
				// check for heavy attack damage modifier
				if (CurrentlyEquippedBy->bHeavyAttacking)
				{
					UGameplayStatics::ApplyDamage(Enemy, BaseHeavyDamage, WeaponInstigator, this, DamageTypeClass);
				}

				// if not, is light attack; apply default damage
				else
				{
					UGameplayStatics::ApplyDamage(Enemy, BaseDamage, WeaponInstigator, this, DamageTypeClass);
				}
			}
		}
	}
}


void AMeleeWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}


void AMeleeWeapon::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}


void AMeleeWeapon::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


