// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonCharacterBase.h"

// Sets default values
AFirstPersonCharacterBase::AFirstPersonCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Create the player camera
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->bUsePawnControlRotation = true;

    // Initialize the capsule half height and camera location for crouching and sliding
    StandingCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    FirstPersonCamera->SetRelativeLocation(CameraOffset);
    StandingCameraZOffset = FirstPersonCamera->GetRelativeLocation().Z;
}

// Called when the game starts or when spawned
void AFirstPersonCharacterBase::BeginPlay()
{
	Super::BeginPlay();

    // Create widget and add it to the viewport
    if (MainHudClass != nullptr)
    {
        MainHudRef = CreateWidget(GetWorld(), MainHudClass);
        MainHudRef->AddToViewport();
    }



}

// Called every frame
void AFirstPersonCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    AFirstPersonCharacterBase::CanStand();
}

// Called to bind functionality to input
void AFirstPersonCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Axis Mappings
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    PlayerInputComponent->BindAxis("MoveForward", this, &AFirstPersonCharacterBase::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AFirstPersonCharacterBase::MoveRight);

    // Action Mappings
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFirstPersonCharacterBase::StartHandleJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &AFirstPersonCharacterBase::StopHandleJump);

    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AFirstPersonCharacterBase::Crouch);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AFirstPersonCharacterBase::BeginSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AFirstPersonCharacterBase::EndSprint);
}

// Handle logic to move the player front to back
void AFirstPersonCharacterBase::MoveForward(float Axis)
{
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    AddMovementInput(Direction, Axis);
}

// Handle logic to move the player side to side
void AFirstPersonCharacterBase::MoveRight(float Axis)
{
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    AddMovementInput(Direction, Axis);
}

// When the jump button is initially pressed
void AFirstPersonCharacterBase::StartHandleJump()
{
    switch (CurrentMovementState) 
    {
        case MovementState::Walking:
            break;
        case MovementState::Sprinting:
            break;
        case MovementState::Crouching:
            AFirstPersonCharacterBase::SetMovementState(MovementState::Walking);
            bCrouchKeyDown = false;
            break;
        case MovementState::Sliding:
            if (bSprintKeyDown)
            {
                AFirstPersonCharacterBase::SetMovementState(MovementState::Sprinting);
                // Maybe Launch Character
                bCrouchKeyDown = false;
            }
            else 
            {
                AFirstPersonCharacterBase::SetMovementState(MovementState::Walking);
                bCrouchKeyDown = false;
            }
            break;
    }
    ACharacter::Jump();

}

// Stop jump when player releases jump button
void AFirstPersonCharacterBase::StopHandleJump()
{
    ACharacter::StopJumping();
}

// Begin Crouching
void AFirstPersonCharacterBase::Crouch()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Crouching"));
    CurrentMovementState = MovementState::Crouching;
    AFirstPersonCharacterBase::TempCrouch();
}

// Handle start Sprint when sprint key pressed
void AFirstPersonCharacterBase::BeginSprint()
{
    bSprintKeyDown = true;

    switch (CurrentMovementState)
    {
    case MovementState::Walking:
        AFirstPersonCharacterBase::SetMovementState(MovementState::Sprinting);
        break;
    case MovementState::Crouching:
        AFirstPersonCharacterBase::SetMovementState(MovementState::Sprinting);
        break;
    }
}

// Handle end sprint when sprint key released
void AFirstPersonCharacterBase::EndSprint()
{
    bSprintKeyDown = false;

    switch (CurrentMovementState)
    {
    case MovementState::Sprinting:
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("RESOLVE"));
        AFirstPersonCharacterBase::ResolveMovementState();
        break;
    }
}

// Set the movement state after a change of movement state
void AFirstPersonCharacterBase::SetMovementState(TEnumAsByte<MovementState> NewMovementState)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("SetMovementMode"));
    if (NewMovementState != CurrentMovementState)
    {
        PreviousMovementState = CurrentMovementState;
        CurrentMovementState = NewMovementState;
        AFirstPersonCharacterBase::OnMovementStateChange(PreviousMovementState);

        switch (CurrentMovementState)
        {
        case MovementState::Walking:
            AFirstPersonCharacterBase::EndCrouch();
            break;
        case MovementState::Sprinting:
            AFirstPersonCharacterBase::EndCrouch();
            break;
        case MovementState::Crouching:
            AFirstPersonCharacterBase::BeginCrouch();
            break;
        case MovementState::Sliding:
            AFirstPersonCharacterBase::BeginCrouch();
            AFirstPersonCharacterBase::BeginSlide();
            break;
        }
    }
}

// Process the movement state based on various previous movement states
void AFirstPersonCharacterBase::OnMovementStateChange(TEnumAsByte<MovementState> InPreviousMovementState)
{
    // Set Max Walk Speed based on previous movement mode
    switch (CurrentMovementState)
    {
    case MovementState::Walking:
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        break;
    case MovementState::Sprinting:
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
        break;
    case MovementState::Crouching:
        GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
        break;
    case MovementState::Sliding:
        GetCharacterMovement()->MaxWalkSpeed = 0.0f;
        break;
    }

    switch (CurrentMovementState)
    {
    case MovementState::Sliding:
        // While sliding
        GetCharacterMovement()->GroundFriction = 8.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
        AFirstPersonCharacterBase::EndSlide();

        // End Slide reset slide variables
        GetCharacterMovement()->Velocity = ((GetActorForwardVector() * SprintSpeed) * SlideMultiplier);

        GetCharacterMovement()->GroundFriction = 0.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 300.0f;
        break;
    }
}

// Check if player can stand and resolve movement
void AFirstPersonCharacterBase::ResolveMovementState()
{
    if (!AFirstPersonCharacterBase::CanStand())
    {
        AFirstPersonCharacterBase::SetMovementState(MovementState::Walking);
    }
    else
    {
        AFirstPersonCharacterBase::SetMovementState(MovementState::Crouching);
    }
}

bool AFirstPersonCharacterBase::CanStand()
{
    if (bCrouchKeyDown)
    {
        return false;
    }

    FVector Loc;
    FRotator Rot;
    FHitResult Hit;

    GetController()->GetPlayerViewPoint(Loc, Rot);

    FVector Start = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    FVector End = FVector(Start.X, Start.Y, (StandingCapsuleHalfHeight * 2.0f) + Start.Z);

    FCollisionQueryParams TraceParams;
    return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);

    // DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 5.0f);

    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Draw!"));
}


