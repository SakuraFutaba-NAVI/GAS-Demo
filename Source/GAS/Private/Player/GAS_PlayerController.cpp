#include "Player/GAS_PlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Character/GAS_BaseCharacter.h"
#include "GameFramework/Character.h"
#include "Bag/InventoryWidget.h"
#include "GameplayTags/GASTags.h"
#include "Kismet/GameplayStatics.h"



//输入功能
void AGAS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// 获取当前玩家控制器的输入子系统，并将跳跃输入动作绑定到Jump函数上。当玩家按下跳跃键时，Jump函数将被调用，从而触发角色的跳跃行为。
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	// 检查输入子系统是否有效，如果无效则直接返回，避免后续操作导致错误。
	if (!IsValid(InputSubsystem)) return;

	// 将存储在InputMappingContexts数组中的输入映射上下文添加到输入子系统中，以确保这些上下文中的输入配置能够生效。
	for (UInputMappingContext* Context : InputMappingContexts)
	{
		InputSubsystem->AddMappingContext(Context, 0);
	}

	// 将Action输入动作绑定到函数上，使得当玩家按下与Action相关联的输入时，函数将被调用。
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

	// 检查增强输入组件和跳跃输入动作是否有效，如果无效则直接返回，避免后续操作导致错误。
	if (!IsValid(EnhancedInputComponent)) return;

	// 将Action输入动作绑定到函数上，使得当玩家按下与Action相关联的输入时，函数将被调用。
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AGAS_PlayerController::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AGAS_PlayerController::StopJumping);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGAS_PlayerController::Look);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGAS_PlayerController::Move);

	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AGAS_PlayerController::Attack);
	EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &AGAS_PlayerController::Skill);
	EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Started, this, &AGAS_PlayerController::Ultimate);
	
	EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &AGAS_PlayerController::ToggleInventory);
}



void AGAS_PlayerController::Jump()
{
	if (!IsValid(GetCharacter())) return;
	if (!IsAlive()) return;
	if (IsAttack()) return;
	
	GetCharacter()->Jump();
}

void AGAS_PlayerController::StopJumping()
{
	if (!IsValid(GetCharacter())) return;
	
	GetCharacter()->StopJumping();
}

void AGAS_PlayerController::Move(const FInputActionValue& Value)
{
	if (!IsValid(GetCharacter())) return;
    if (!IsAlive()) return;
	if (IsAttack()) return;
	
	// 从输入动作值中获取一个二维向量，表示玩家的移动输入。这个向量通常包含水平和垂直方向的输入值，用于控制角色的移动。
	const FVector2D MoveValue = Value.Get<FVector2D>();

	// 获取当前控制器的旋转，并创建一个新的旋转对象YawRotation，其中只保留了控制器的偏航（水平旋转）部分。这是为了确保角色的移动方向与玩家当前的视角方向一致。
	const FRotator YawRotation = FRotator(0, GetControlRotation().Yaw, 0);
	// 使用旋转矩阵从YawRotation中提取出前向方向（ForwardDirection）和右向方向（RightDirection）。这些方向向量将用于计算角色的移动方向。
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 根据玩家的输入值计算角色的移动方向，并调用AddMovementInput函数将移动输入应用到角色上。
	GetCharacter()->AddMovementInput(ForwardDirection, MoveValue.Y);
	GetCharacter()->AddMovementInput(RightDirection, MoveValue.X);
}

void AGAS_PlayerController::Look(const FInputActionValue& Value)
{
	if (!IsValid(GetCharacter())) return;

	// 从输入动作值中获取一个二维向量，表示玩家的视角输入。这个向量通常包含水平和垂直方向的输入值，用于控制角色的视角旋转。
	const FVector2D LookValue = Value.Get<FVector2D>();
	
	// 调用函数将视角输入应用到控制器上，从而实现角色的视角旋转。水平输入（LookValue.X）用于控制偏航（Yaw），垂直输入（LookValue.Y）用于控制俯仰（Pitch）。
	AddYawInput(LookValue.X);
	AddPitchInput(LookValue.Y);
}

void AGAS_PlayerController::Attack()
{
	AGAS_BaseCharacter* BaseChar = Cast<AGAS_BaseCharacter>(GetPawn());
	if (IsValid(BaseChar))
	{
		BaseChar->BeginAttack(); 
	}
	
	ActivateAbility(GASTags::GASAbilities::Attack);
}

void AGAS_PlayerController::Skill()
{
	ActivateAbility(GASTags::GASAbilities::Skill);
}

void AGAS_PlayerController::Ultimate()
{
	ActivateAbility(GASTags::GASAbilities::Ultimate);
}

void AGAS_PlayerController::ActivateAbility(const FGameplayTag& AbilityTag) const
{
	if (!IsAlive()) return;
	if (IsJump()) return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn());
	if (!IsValid(ASC)) return;
	
	ASC->TryActivateAbilitiesByTag(AbilityTag.GetSingleTagContainer());
}



bool AGAS_PlayerController::IsAlive() const
{
	AGAS_BaseCharacter* BaseCharacter = Cast<AGAS_BaseCharacter>(GetPawn());
	if (!IsValid(BaseCharacter)) return false;
	return BaseCharacter->IsAlive();
}

bool AGAS_PlayerController::IsMove() const
{
	AGAS_BaseCharacter* BaseCharacter = Cast<AGAS_BaseCharacter>(GetPawn());
	if (!IsValid(BaseCharacter)) return false;
	return BaseCharacter->IsMove();
}

bool AGAS_PlayerController::IsAttack() const
{
	AGAS_BaseCharacter* BaseCharacter = Cast<AGAS_BaseCharacter>(GetPawn());
	if (!IsValid(BaseCharacter)) return false;
	return BaseCharacter->IsAttack();
}

bool AGAS_PlayerController::IsJump() const
{
	AGAS_BaseCharacter* BaseCharacter = Cast<AGAS_BaseCharacter>(GetPawn());
	if (!IsValid(BaseCharacter)) return false;
	return BaseCharacter->IsJump();
}

void AGAS_PlayerController::OnMoveStarted(const FInputActionValue& Value)
{
	if (AGAS_BaseCharacter* BaseCharacter = Cast<AGAS_BaseCharacter>(GetCharacter()))
	{
		BaseCharacter->SetMove(true);
	}
}

void AGAS_PlayerController::OnMoveCompleted(const FInputActionValue& Value)
{
	if (AGAS_BaseCharacter* BaseCharacter = Cast<AGAS_BaseCharacter>(GetCharacter()))
	{
		BaseCharacter->SetMove(false);
	}
}


// ===================== 背包开关逻辑 =====================
void AGAS_PlayerController::ToggleInventory()
{
	if (bIsInventoryOpen)
	{
		CloseInventory();
	}
	else
	{
		OpenInventory();
	}
}

void AGAS_PlayerController::OpenInventory()
{
	AGAS_BaseCharacter* BaseCharacter = Cast<AGAS_BaseCharacter>(GetPawn());
	if (!InventoryWidgetClass || !BaseCharacter) return;

	if (!CurrentInventoryWidget)
	{
		CurrentInventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
		CurrentInventoryWidget->InitInventory(BaseCharacter->InventoryComponent);
	}

	CurrentInventoryWidget->AddToViewport();
	bShowMouseCursor = true;
	SetInputMode(FInputModeGameAndUI());

	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.f);
	bIsInventoryOpen = true;
}

void AGAS_PlayerController::CloseInventory()
{
	if (!CurrentInventoryWidget) return;

	CurrentInventoryWidget->RemoveFromParent();
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	// 恢复游戏
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
	bIsInventoryOpen = false;
}