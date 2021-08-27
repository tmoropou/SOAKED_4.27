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

    ClimbHeight = CreateDefaultSubobject<USceneComponent>(TEXT("Climb Height"));
    ClimbHeight->SetupAttachment(GetCapsuleComponent());

    CurrentMovementState = MovementState::Walking;
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
    const FVector Direction2 = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Z);
    if (GetCharacterMovement()->IsFlying())
    {
        AddMovementInput(Direction2, Axis);
    }
    else
    {
        AddMovementInput(Direction, Axis);
    }

}

// Handle logic to move the player side to side
void AFirstPersonCharacterBase::MoveRight(float Axis)
{
    if (GetCharacterMovement()->IsFlying())
    {

    }
    else
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Axis);
    }
}

// When the jump button is initially pressed
void AFirstPersonCharacterBase::StartHandleJump()
{
    ACharacter::Jump();

    
    switch (CurrentMovementState) 
    {
        case MovementState::Walking:
            break;
        case MovementState::Sprinting:
            GetWorldTimerManager().SetTimer(CheckClimbTimerHandle, this, &AFirstPersonCharacterBase::CheckClimbTimer, 0.1f, true, 0.0f);
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

}

// Stop jump when player releases jump button
void AFirstPersonCharacterBase::StopHandleJump()
{
    ACharacter::StopJumping();
}

// Begin Crouching
void AFirstPersonCharacterBase::Crouch()
{

    switch (CurrentMovementState)
    {
    case MovementState::Walking:
        AFirstPersonCharacterBase::SetMovementState(MovementState::Crouching);
        break;
    case MovementState::Sprinting:
        AFirstPersonCharacterBase::SetMovementState(MovementState::Sliding);
        break;
    case MovementState::Crouching:
        if (AFirstPersonCharacterBase::CanStand())
        {
            bCrouchKeyDown = false;
            AFirstPersonCharacterBase::EndCrouch();
            AFirstPersonCharacterBase::ResolveMovementState();
            CurrentMovementState = MovementState::Walking;
        }
        break;
    }
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
        if (AFirstPersonCharacterBase::CanStand())
        {
            AFirstPersonCharacterBase::EndCrouch();
            AFirstPersonCharacterBase::SetMovementState(MovementState::Sprinting);
        }
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
        AFirstPersonCharacterBase::ResolveMovementState();
        break;
    }
}

// Set the movement state after a change of movement state
void AFirstPersonCharacterBase::SetMovementState(TEnumAsByte<MovementState> NewMovementState)
{
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
            AFirstPersonCharacterBase::BeginCameraTilt();
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

    // End slide
    switch (PreviousMovementState)
    {
    case MovementState::Sliding:
        GetWorldTimerManager().ClearTimer(SlideTimerHandle);
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Sliding"));
        GetCharacterMovement()->GroundFriction = 8.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
        AFirstPersonCharacterBase::EndCameraTilt();
        break;
    }

    // Start Slide
    switch (CurrentMovementState)
    {
    case MovementState::Sliding:
        GetWorldTimerManager().SetTimer(SlideTimerHandle, this, &AFirstPersonCharacterBase::SlideTimer, 0.1f, true, 0.0f);
        GetCharacterMovement()->Velocity = ((GetActorForwardVector() * SprintSpeed));
        GetCharacterMovement()->GroundFriction = 0.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 300.0f;
        break;
    }
}

// Check if player can stand and resolve movement
void AFirstPersonCharacterBase::ResolveMovementState()
{
    switch (CurrentMovementState)
    {
        case MovementState::Walking:
            if (AFirstPersonCharacterBase::CanStand())
            {
                AFirstPersonCharacterBase::SetMovementState(MovementState::Walking);
            }
            else
            {
                AFirstPersonCharacterBase::SetMovementState(MovementState::Crouching);
            }
            break;

        case MovementState::Sprinting:
            if (AFirstPersonCharacterBase::CanStand())
            {
                AFirstPersonCharacterBase::SetMovementState(MovementState::Walking);
            }
            else
            {
                AFirstPersonCharacterBase::SetMovementState(MovementState::Crouching);
            }
            break;

        case MovementState::Crouching:
            if (AFirstPersonCharacterBase::CanStand())
            {
                AFirstPersonCharacterBase::SetMovementState(MovementState::Walking);
            }
            else
            {
                AFirstPersonCharacterBase::SetMovementState(MovementState::Crouching);
            }
            break;

        case MovementState::Sliding:
            AFirstPersonCharacterBase::EndSlide();
            AFirstPersonCharacterBase::SetMovementState(MovementState::Crouching);
            break;
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
    return !GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);

    // DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 5.0f);

    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Draw!"));
}

FVector AFirstPersonCharacterBase::CalculateFloorInfluence(FVector FloorNormal)
{
    FVector VectorUp = FVector(0.0f, 0.0f, 1.0f);
    FVector FinalVector = FVector(0.0f, 0.0f, 0.0f);
    float SecondMultiple = 0.0f;
    if (FloorNormal == VectorUp)
    {
        return FVector(0.0f, 0.0f, 0.0f);
    } 
    else
    {
        // Handle Final vector calculations with the floor normal
        FinalVector = FVector::CrossProduct(FloorNormal, VectorUp);
        FinalVector = FVector::CrossProduct(FloorNormal, FinalVector);
        FinalVector.Normalize();

        // Handle the multiplier with the floor normal to be multiplied with the FinalVector
        SecondMultiple = FVector::DotProduct(FloorNormal, VectorUp);
        SecondMultiple = 1.0f - SecondMultiple;
        SecondMultiple = FMath::Clamp(SecondMultiple, 0.0f, 1.0f);
        SecondMultiple = SecondMultiple * 500000.0f;

        FinalVector = FinalVector * SecondMultiple;
    }
    return FVector();
}

void AFirstPersonCharacterBase::SlideTimer()
{
    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Sliding"));
    FVector FloorNormal = GetCharacterMovement()->CurrentFloor.HitResult.Normal;
    GetCharacterMovement()->AddForce(AFirstPersonCharacterBase::CalculateFloorInfluence(FloorNormal));

    if (GetVelocity().Size() > SprintSpeed)
    {
        FVector MyVec = GetVelocity();
        MyVec.Normalize();
        MyVec = MyVec * SprintSpeed;
        GetCharacterMovement()->Velocity = MyVec;
    }

    if (GetVelocity().Size() < CrouchSpeed)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Too Slow!"));
        AFirstPersonCharacterBase::ResolveMovementState();
    }
}

bool AFirstPersonCharacterBase::LineTrace(FHitResult Hit, FVector Start, FVector End)
{
    FCollisionQueryParams TraceParams;
    DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 10.0f);
    return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);
}

void AFirstPersonCharacterBase::ClimbTimer()
{
    FHitResult Hit;
    FVector Start = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - 100);
    FVector End = FVector((GetActorForwardVector() * 75) + Start);

    FHitResult Hit2;
    FVector Start2 = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 90);
    FVector End2 = FVector((GetActorForwardVector() * 75) + Start2);

    if (!LineTrace(Hit2, Start2, End2))
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Finished Climbing"));
        bFinishClimbing = true;
    }

    if (LineTrace(Hit, Start, End))
    {

    }
    else 
    {
        bFinishClimbing = false;
        bClimbing = false;
        ClimbIteration = 0;
        GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
        GetWorldTimerManager().ClearTimer(ClimbTimerHandle);
        GetCharacterMovement()->MaxFlySpeed = 200;
    }

    if (ClimbIteration > 10)
    {
        bFinishClimbing = false;
        bClimbing = false;
        ClimbIteration = 0;
        GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
        GetWorldTimerManager().ClearTimer(ClimbTimerHandle);
        GetCharacterMovement()->MaxFlySpeed = 200;
    }

    ClimbIteration = ClimbIteration + 1;
}

void AFirstPersonCharacterBase::CheckClimbTimer()
{
    FHitResult Hit;
    FVector Start = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z - 60);
    FVector End = FVector((GetActorForwardVector() * 100) + Start);

    if (AFirstPersonCharacterBase::LineTrace(Hit, Start, End))
    {
        bClimbing = true;
        GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
        GetWorldTimerManager().ClearTimer(CheckClimbTimerHandle);
        GetWorldTimerManager().SetTimer(ClimbTimerHandle, this, &AFirstPersonCharacterBase::ClimbTimer, 0.1f, true, 0.0f);
        GetCharacterMovement()->MaxFlySpeed = 300;
    }

    if (!GetCharacterMovement()->IsFalling())
    {
        GetWorldTimerManager().ClearTimer(CheckClimbTimerHandle);
    }
}
