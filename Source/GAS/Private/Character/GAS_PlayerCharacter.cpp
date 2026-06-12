#include "Character/GAS_PlayerCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Player/GAS_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GAS_AttributeSet.h"


AGAS_PlayerCharacter::AGAS_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f); // 初始化胶囊组件的大小，设置半径，高度

	bUseControllerRotationPitch = false; // 不允许角色随控制器的俯仰旋转
	bUseControllerRotationYaw = false; // 不允许角色随控制器的偏航旋转
	bUseControllerRotationPitch = false; // 不允许角色随控制器的滚转旋转

	GetCharacterMovement()->bOrientRotationToMovement = true; // 让角色的朝向随移动方向自动调整
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // 设置角色旋转的速率，偏航旋转速率为
	GetCharacterMovement()->JumpZVelocity = 600.f; // 设置角色跳跃的垂直速度
	GetCharacterMovement()->AirControl = 0.3f; // 设置角色在空中时的控制能力
	GetCharacterMovement()->MaxWalkSpeed = 600.f; // 设置角色的最大行走速度
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f; // 设置角色的最小模拟行走速度
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f; // 设置角色在行走时的制动减速度
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f; // 设置角色在下落时的制动减速度
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");// 创建一个USpringArmComponent组件，并将其命名为"CameraBoom"
	CameraBoom->SetupAttachment(RootComponent);// 将CameraBoom附加到角色的根组件上
	CameraBoom->TargetArmLength = 600.0f; // 设置相机与角色之间的距离
	CameraBoom->bUsePawnControlRotation = true; // 让相机随角色旋转

	FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");// 创建一个UCameraComponent组件，并将其命名为"FollowCamera"
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 将FollowCamera附加到CameraBoom的末端
	FollowCamera->bUsePawnControlRotation = false; // 不让相机随角色旋转
	
	
	Tags.Add(GAS_Tags::Player);
}


//获取能力组件
UAbilitySystemComponent* AGAS_PlayerCharacter::GetAbilitySystemComponent() const
{
	// 从角色的PlayerState获取AbilitySystemComponent组件的指针并转换为AGAS_PlayerState类型。GetAbilitySystemComponent函数返回AbilitySystemComponent组件的指针。
	AGAS_PlayerState* GASPlayerState = Cast<AGAS_PlayerState>(GetPlayerState());

	if (!IsValid(GASPlayerState)) return nullptr;
	return GASPlayerState->GetAbilitySystemComponent();
}


//获取属性集
UAttributeSet* AGAS_PlayerCharacter::GetAttributeSet() const
{
	AGAS_PlayerState* GASPlayerState = Cast<AGAS_PlayerState>(GetPlayerState());

	if (!IsValid(GASPlayerState)) return nullptr;
	return GASPlayerState->GetAttributeSet();
}



void AGAS_PlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!IsValid(GetAbilitySystemComponent())||!HasAuthority()) return;

	// 初始化AbilitySystemComponent的Actor信息，传入PlayerState和角色本身作为参数。在角色被控制器占领时调用，确保AbilitySystemComponent组件正确地关联到角色和玩家状态。
	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
    
	OnASCInitialized.Broadcast(GetAbilitySystemComponent(),GetAttributeSet());
	
	GiveStartupAbilities();
	InitializeAttributes();
	
	UGAS_AttributeSet* GAS_AttributeSet = Cast<UGAS_AttributeSet>(GetAttributeSet());
	if (!IsValid(GAS_AttributeSet)) return;
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(GAS_AttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChange);
}



void AGAS_PlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (!IsValid(GetAbilitySystemComponent())) return;

	// 当PlayerState发生变化时调用，确保AbilitySystemComponent组件正确地关联到新的玩家状态和角色本身。这对于在网络环境中保持角色状态的一致性非常重要。
	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);

	OnASCInitialized.Broadcast(GetAbilitySystemComponent(),GetAttributeSet());
}
