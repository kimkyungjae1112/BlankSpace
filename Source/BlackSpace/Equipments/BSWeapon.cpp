// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipments/BSWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

#include "Components/BSCombatComponent.h"
#include "Components/BSWeaponCollisionComponent.h"
#include "BSGameplayTag.h"
#include "Data/BSMontageActionData.h"
#include "Interface/BSUpdateAnyTypeInterface.h"

ABSWeapon::ABSWeapon()
{
	InventoryInfo.ItemClass = StaticClass();

	CollisionComp = CreateDefaultSubobject<UBSWeaponCollisionComponent>(TEXT("Collision Component"));
	CollisionComp->OnHitActor.AddUObject(this, &ThisClass::OnHitActor);

	SecondaryCollisionComp = CreateDefaultSubobject<UBSWeaponCollisionComponent>(TEXT("Secondary Collision Component"));
	SecondaryCollisionComp->OnHitActor.AddUObject(this, &ThisClass::OnHitActor);

	StaminaCosts.Add(BSGameplayTag::Character_Attack_Light, 5.f);
	StaminaCosts.Add(BSGameplayTag::Character_Attack_Heavy, 7.f);
	StaminaCosts.Add(BSGameplayTag::Character_Attack_Running, 6.f);
	StaminaCosts.Add(BSGameplayTag::Character_Attack_Special, 10.f);
}

void ABSWeapon::EquipItem()
{
	Super::EquipItem();

	CombatComp = GetOwner()->GetComponentByClass<UBSCombatComponent>();
	if (CombatComp)
	{
		// 메인, 보조 무기가 모두 있을 때, 메인 무기 탈부착
		if (CombatComp->CheckHasMainWeapon() && CombatComp->CheckHasSecondaryWeapon())
		{
			AttachToOwner(EquipSocket);
		}
		// 메인 무기가 없으면 메인 무기 부착
		else if (CombatComp->CheckHasMainWeapon() == false)
		{
			AttachToOwner(EquipSocket);
		}

		CombatComp->SetWeapon(this);

		CollisionComp->SetWeaponMesh(MeshComp);

		CollisionComp->AddIgnoreActor(GetOwner());

		SecondaryCollisionComp->SetWeaponMesh(MeshComp);

		SecondaryCollisionComp->AddIgnoreActor(GetOwner());
	}
}

void ABSWeapon::UnequipItem()
{
	Super::UnequipItem();

	if (ACharacter* Player = Cast<ACharacter>(GetOwner()))
	{
		if (IBSUpdateAnyTypeInterface* AnimInterface = Cast<IBSUpdateAnyTypeInterface>(Player->GetMesh()->GetAnimInstance()))
		{
			AnimInterface->UpdateWeaponType(EWeaponType::Uncombat);
		}
	}

	Destroy();
}

void ABSWeapon::OnUpdateWeaponType()
{
	if (ACharacter* Player = Cast<ACharacter>(GetOwner()))
	{
		if (IBSUpdateAnyTypeInterface* AnimInterface = Cast<IBSUpdateAnyTypeInterface>(Player->GetMesh()->GetAnimInstance()))
		{
			AnimInterface->UpdateWeaponType(WeaponType);
		}
	}
}

UAnimMontage* ABSWeapon::GetMontageForTag(const FGameplayTag& GroupTag, const int32 Index) const
{
	return MontageData->GetMontageForTag(GroupTag, Index);
}

float ABSWeapon::GetStaminaCost(const FGameplayTag& InAttackType) const
{
	if (StaminaCosts.Contains(InAttackType))
	{
		return StaminaCosts[InAttackType];
	}
	return -1.f;
}

float ABSWeapon::GetAttackDamage() const
{
	return BaseDamage;
}

void ABSWeapon::ActivateWeaponCollision(const EWeaponCollisionType& WeaponCollisionType)
{
	switch (WeaponCollisionType)
	{
	case EWeaponCollisionType::First:
		CollisionComp->TurnOnCollision();
		break;
	case EWeaponCollisionType::Second:
		SecondaryCollisionComp->TurnOnCollision();
		break;
	}
}

void ABSWeapon::DeactivateWeaponCollision(const EWeaponCollisionType& WeaponCollisionType)
{
	switch (WeaponCollisionType)
	{
	case EWeaponCollisionType::First:
		CollisionComp->TurnOffCollision();
		break;
	case EWeaponCollisionType::Second:
		SecondaryCollisionComp->TurnOffCollision();
		break;
	}
}

void ABSWeapon::OnHitActor(const FHitResult& HitResult)
{
	AActor* TargetActor = HitResult.GetActor();

	float AttackDamage = GetAttackDamage();

	const FVector DamageDirection = GetOwner()->GetActorForwardVector();

	UGameplayStatics::ApplyPointDamage(
		TargetActor,
		AttackDamage,
		DamageDirection,
		HitResult,
		GetOwner()->GetInstigatorController(),
		this,
		nullptr
	);
}

