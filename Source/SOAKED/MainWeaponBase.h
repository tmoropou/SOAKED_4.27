// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "FirstPersonCharacterBase.h"
#include "MainWeaponBase.generated.h"

UCLASS()
class SOAKED_API AMainWeaponBase : public AActor
{
	GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int WaterLevel = 100;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bCoolDown = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    AFirstPersonCharacterBase* PlayerRef;


public:	
	// Sets default values for this actor's properties
	AMainWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
