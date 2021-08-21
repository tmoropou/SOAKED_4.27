// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "MovmentStateEnum.generated.h"

/**
 * 
 */

UCLASS()
class SOAKED_API UMovmentStateEnum : public UUserDefinedEnum
{
	GENERATED_BODY()
	
};

UENUM(BlueprintType)
enum MovementState
{
    Walking       UMETA(DisplayName = "Walking"),
    Sprinting     UMETA(DisplayName = "Sprinting"),
    Crouching     UMETA(DisplayName = "Crouching"),
    Sliding       UMETA(DisplayName = "Sliding"),
};
