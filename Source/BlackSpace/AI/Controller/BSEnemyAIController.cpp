// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controller/BSEnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"

#include "Characters/BSCharacterEnemy.h"
#include "Characters/BSCharacterPlayer.h"
#include "Components/BSRotationComponent.h"

ABSEnemyAIController::ABSEnemyAIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"));
}

bool ABSEnemyAIController::IsDetectedPlayer() const
{
	return (Blackboard->GetValueAsObject(TEXT("Target")) != nullptr) ? true : false;
}

float ABSEnemyAIController::ToTargetDist() const
{
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TEXT("Target")));
	if (IsValid(Target))
	{
		return GetPawn()->GetDistanceTo(Target);
	}

	return -1.f;
}

void ABSEnemyAIController::StopUpdateTarget()
{
	GetWorld()->GetTimerManager().ClearTimer(PerceptionTimerHandle);
	SetTarget(nullptr);
}

void ABSEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledEnemy = Cast<ABSCharacterEnemy>(InPawn);

	RunBehaviorTree(BehaviorTreeAsset);

	GetWorld()->GetTimerManager().SetTimer(PerceptionTimerHandle, this, &ThisClass::UpdateTarget, 0.1f, true);
}

void ABSEnemyAIController::OnUnPossess()
{
	ControlledEnemy = nullptr;
	StopUpdateTarget();
	Super::OnUnPossess();
}

void ABSEnemyAIController::UpdateTarget() const
{
	TArray<AActor*> OutActors;
	AIPerceptionComp->GetKnownPerceivedActors(nullptr, OutActors);

	ABSCharacterPlayer* PlayerCharacter = Cast<ABSCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!IsValid(PlayerCharacter)) return;

	if (OutActors.Contains(PlayerCharacter))
	{
		if (!PlayerCharacter->IsDead())
		{
			if (!IsDetectedPlayer())
			{
				ControlledEnemy->SeesTarget(PlayerCharacter);
			}

			SetTarget(PlayerCharacter);
			ControlledEnemy->ToggleHealthBarVisibility(true);
		}
		else
		{
			SetTarget(nullptr);
			ControlledEnemy->ToggleHealthBarVisibility(false);
		}
	}
	else
	{
		SetTarget(nullptr);
		ControlledEnemy->ToggleHealthBarVisibility(false);
	}

}

void ABSEnemyAIController::SetTarget(AActor* NewTarget) const
{
	if (IsValid(Blackboard))
	{
		Blackboard->SetValueAsObject(TEXT("Target"), NewTarget);
	}

	if (IsValid(ControlledEnemy))
	{
		if (UBSRotationComponent* RotationComp = ControlledEnemy->GetComponentByClass<UBSRotationComponent>())
		{
			RotationComp->SetTargetActor(NewTarget);
		}
	}
	
}
