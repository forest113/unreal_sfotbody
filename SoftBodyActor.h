// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "SoftBodyActor.generated.h"


enum mesh_type { ico, prism, cube };
struct PointMass {
	/* Vertice that the point mass is sitting on.*/
	FVector vert;
	FVector impulse = FVector(0, 0, 0);
	FVector vel;
	int ind; //debug
};

struct Spring {
	float rest_len;
	float cur_len;
	/* The masses at the ends of the spring*/
	PointMass *pm1, *pm2;
};

UCLASS()
class MYFIRSTPLUGIN_API ASoftBodyActor : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* mesh;
	mesh_type type = prism;
	/* Point masses in the spring-mass model*/
	TArray<PointMass> Points;
	TArray<int32> Triangles;
	TArray<Spring> Springs;
	
public:	
	// Sets default values for this actor's properties
	ASoftBodyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void PostActorCreated() override;
	//void CreateTriangle(TArray<FVector> vertices);
	void CreateIco();
	void CreateSpringsIco(TArray<int32> tris);
	void CreatePrism();
	void CreateSpringsPrism();
	void CreateCube();
	void CreateSpringsCube();
	double GetSurfaceArea();
	void ResetImpulses();
	
	void ProcessPointMasses(float DeltaTime);
	void ProcessSprings(float DeltaTime);
	void UpdateMesh();
};
