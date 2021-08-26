// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include "MovmentStateEnum.h"
#include "FirstPersonCharacterBase.generated.h"

UCLASS()
class SOAKED_API AFirstPersonCharacterBase : public ACharacter
{
	GENERATED_BODY()

protected: 
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyCharacter(C++)")
        float StandingCapsuleHalfHeight = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyCharacter(C++)")
        float StandingCameraZOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyCharacter(C++)")
        FVector CameraOffset = FVector(0.0f, 0.0f, 50.0f);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyCharacter(C++)")
        float WalkSpeed = 450.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyCharacter(C++)")
        float SprintSpeed = 600.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyCharacter(C++)")
        float CrouchSpeed = 150.0f;

    UPROPERTY(EditAnywhere, Category = "HUD(C++)")
        TSubclassOf<UUserWidget> MainHudClass;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "HUD(C++)")
        UUserWidget* MainHudRef;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components (C++)")
        UCameraComponent* FirstPersonCamera;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components (C++)")
        USceneComponent* ClimbHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enums(C++)")
        TEnumAsByte<MovementState> CurrentMovementState;
        TEnumAsByte<MovementState> PreviousMovementState;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterBools(C++)")
        bool bCrouchKeyDown = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterBools(C++)")
        bool bSprintKeyDown = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CharacterBools(C++)")
        bool bClimbingLadder = false;

    void MoveForward(float Axis);
    void MoveRight(float Axis);

    UFUNCTION(BlueprintCallable, Category = "Movement(C++)")
        void StartHandleJump();
        void StopHandleJump();
        void Crouch();
        void BeginSprint();
        void EndSprint();
        void SetMovementState(TEnumAsByte<MovementState> NewMovementState);
        void OnMovementStateChange(TEnumAsByte<MovementState> InPreviousMovementState);
        void ResolveMovementState();
        bool CanStand();
        bool LineTrace(FHitResult Hit, FVector Start, FVector End);

    UFUNCTION(BlueprintImplementableEvent, Category = "C++ Functions")
        void TempCrouch();

    UFUNCTION(BlueprintImplementableEvent, Category = "C++ Functions")
        void BeginCrouch();

    UFUNCTION(BlueprintImplementableEvent, Category = "C++ Functions")
        void EndCrouch();

    UFUNCTION(BlueprintImplementableEvent, Category = "C++ Functions")
        void BeginSlide();

    UFUNCTION(BlueprintImplementableEvent, Category = "C++ Functions")
        void EndSlide();

    UFUNCTION(BlueprintImplementableEvent, Category = "C++ Functions")
        void BeginCameraTilt();

    UFUNCTION(BlueprintImplementableEvent, Category = "C++ Functions")
        void EndCameraTilt();

    UFUNCTION()
        FVector CalculateFloorInfluence(FVector FloorNormal);

    UFUNCTION()
        void SlideTimer();

    FTimerHandle SlideTimerHandle;

    UFUNCTION()
        void ClimbTimer();

    FTimerHandle ClimbTimerHandle;

public:
	// Sets default values for this character's properties
	AFirstPersonCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
