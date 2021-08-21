// Fill out your copyright notice in the Description page of Project Settings.


#include "MainWeaponBase.h"

// Sets default values
AMainWeaponBase::AMainWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    PlayerRef = Cast<AFirstPersonCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

// Called when the game starts or when spawned
void AMainWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMainWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

