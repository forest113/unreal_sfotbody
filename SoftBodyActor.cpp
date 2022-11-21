// Fill out your copyright notice in the Description page of Project Settings.


#include "SoftBodyActor.h"

#define X_coord 10
#define Z_coord 16
#define PM_MASS 0.1
#define SPRING_K 0.03
#define SPRING_DAMP 0.0001
#define GRAVITY 10
#define RESTITUTION 0.9



FVector ICO_VERTS[12] = {
   {-X_coord, 0.0, Z_coord}, { X_coord, 0.0, Z_coord }, { -X_coord, 0.0, -Z_coord }, { X_coord, 0.0, -Z_coord },
   { 0.0, Z_coord, X_coord }, { 0.0, Z_coord, -X_coord }, { 0.0, -Z_coord, X_coord }, { 0.0, -Z_coord, -X_coord },
   { Z_coord, X_coord, 0.0 }, { -Z_coord, X_coord, 0.0 }, { Z_coord, -X_coord, 0.0 }, { -Z_coord, -X_coord, 0.0 }
};

int ICO_TRIS[20][3] = {
   {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
   {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
   {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
   {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

// Sets default values
ASoftBodyActor::ASoftBodyActor()
{
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
    // New in UE 4.17, multi-threaded PhysX cooking.
    mesh->bUseAsyncCooking = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	

}

// This is called when actor is spawned (at runtime or when you drop it into the world in editor)
void ASoftBodyActor::PostActorCreated()
{
	Super::PostActorCreated();
	if (type == ico) {
		CreateIco();
	}
	else if (type == prism){
		CreatePrism();
	}
	else if (type == cube) {
		CreateCube();
	}
}



// Called when the game starts or when spawned
void ASoftBodyActor::BeginPlay()
{
	Super::BeginPlay();
	if (type == ico) {
		CreateIco();
	}
	else if (type == prism) {
		CreatePrism();
	}
	else if (type == cube) {
		CreateCube();
	}
}

// Called every frame
void ASoftBodyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UpdateTriangle();
	ResetImpulses();
	ProcessSprings(DeltaTime);
	ProcessPointMasses(DeltaTime);
	UpdateMesh();

}

void ASoftBodyActor::CreateIco() {
	// create icosahedron
	TArray<FVector> vertices;
	for (int i = 0; i < 12; i++) {
		PointMass point;
		point.vert = (FVector)ICO_VERTS[i];
		point.impulse = FVector(0, 0, 0);
		point.ind = i;
		point.vel = FVector(0, 0, 0);
		vertices.Add((FVector)ICO_VERTS[i]);
		Points.Add(point);
		
	}
	/* one extra point mass for the center */
	FVector center = mesh->Bounds.GetSphere().Center;
	PointMass point;
	point.vert = center;
	point.impulse = FVector(0, 0, 0);
	point.ind = 12;
	point.vel = FVector(0, 0, 0);
	Points.Add(point);

	for (int i = 0; i < 20; i++) {
		int tri[3];
		for (int j = 0; j < 3; j++) {
			tri[j] = (ICO_TRIS[i][j]);
			Triangles.Add(tri[j]);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Some variable values: x: %f, y: %f z:%f"), vertices[0][0], vertices[0][1], vertices[0][2]));
		//Triangles.Add(tri);
	}

	TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	//VertexColors.Add(FLinearColor(0.f, 0.f, 1.f));
	vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
	vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
	vertexColors.Add(FLinearColor(0.f, 1.f, 0.f));
	vertexColors.Add(FLinearColor(0.5f, 1.f, 0.5f));
	vertexColors.Add(FLinearColor(0.f, 1.f, 0.f));
	vertexColors.Add(FLinearColor(1.f, 1.f, 0.f));

	mesh->CreateMeshSection_LinearColor(1, vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), vertexColors, TArray<FProcMeshTangent>(), true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
	CreateSpringsIco(Triangles);
}



void ASoftBodyActor::CreateSpringsIco(TArray<int32> tris) {
	for (int i = 0; i < tris.Num()/3; i++) {
		PointMass *pm1, *pm2, *pm3;
		pm1 = &(Points[tris[i]]);
		pm2 = &(Points[tris[i + 1]]);
		pm3 = &(Points[tris[i + 2]]);
		float edge_len1 = (pm1->vert - pm2->vert).Size();
		float edge_len2 = (pm2->vert - pm3->vert).Size();
		float edge_len3 = (pm3->vert - pm1->vert).Size();
		Spring s1, s2, s3;
		GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("tri edge len:%f pms: %d %d %d"), edge_len1, pm1->ind, pm2->ind, pm3->ind ));
		s1.rest_len = s1.cur_len = edge_len1;
		s2.rest_len = s2.cur_len = edge_len2;
		s3.rest_len = s3.cur_len = edge_len3;
		s1.pm1 = pm1;
		s1.pm2 = pm2;
		s2.pm1 = pm2;
		s2.pm2 = pm3;
		s3.pm1 = pm3;
		s3.pm2 = pm1;
		Springs.Add(s1);
		Springs.Add(s2);
		Springs.Add(s3);
	}

	/*internal springs */
	for (int i = 0; i < 12; i++) {
		PointMass* pm1, * pm2;
		pm1 = &(Points[i]);
		pm2 = &(Points[12]);
		float edge_len = (pm1->vert - pm2->vert).Size();
		Spring s;
		s.rest_len = s.cur_len = edge_len;
		s.pm1 = pm1;
		s.pm2 = pm2;
		Springs.Add(s);
	}
}

void ASoftBodyActor::CreatePrism() {
	FVector prism_verts[4] = { {5,5,5} , {5,-5,-5}, {-5,-5,5}, {-5,5,-5} };
	int prism_tris[4][3] = { {0,1,2}, {2,1,3}, {3,0,2}, {3,1,0} };
	TArray<FVector> vertices;
	for (int i = 0; i < 4; i++) {
		PointMass point;
		point.vert = (FVector)prism_verts[i];
		point.impulse = FVector(0, 0, 0);
		point.ind = i;
		point.vel = FVector(0, 0, 0);
		vertices.Add((FVector)prism_verts[i]);
		Points.Add(point);
	}
	/* one extra point mass for the center */
	FVector center = FVector(0, 0, 0);
	PointMass point;
	point.vert = center;
	point.impulse = FVector(0, 0, 0);
	point.ind = 12;
	point.vel = FVector(0, 0, 0);
	Points.Add(point);

	for (int i = 0; i < 4; i++) {
		int tri[3];
		for (int j = 0; j < 3; j++) {
			tri[j] = (prism_tris[i][j]);
			Triangles.Add(tri[j]);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Some variable values: x: %f, y: %f z:%f"), vertices[0][0], vertices[0][1], vertices[0][2]));
		//Triangles.Add(tri);
	}
	TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	mesh->CreateMeshSection_LinearColor(1, vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), vertexColors, TArray<FProcMeshTangent>(), true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
	CreateSpringsPrism();
}

void ASoftBodyActor::CreateSpringsPrism() {
	int edges[6][2] = { {0,1},{0,2},{0,3},{1,2},{1,3}, {2,3} };
	for (int i = 0; i < 6; i++) {
		PointMass *pm1, *pm2;
		pm1 = &(Points[edges[i][0]]);
		pm2 = &(Points[edges[i][1]]);
		Spring s;
		s.pm1 = pm1;
		s.pm2 = pm2;
		float edge_len = (pm1->vert - pm2->vert).Size();
		s.rest_len = s.cur_len = edge_len;
		Springs.Add(s);
	}

	/* internal springs */
	for (int i = 0; i < 4; i++) {
		PointMass* pm1, * pm2;
		pm1 = &(Points[i]);
		pm2 = &(Points[4]);
		float edge_len = (pm1->vert - pm2->vert).Size();
		Spring s;
		s.rest_len = s.cur_len = edge_len;
		s.pm1 = pm1;
		s.pm2 = pm2;
		Springs.Add(s);
	}

}

void ASoftBodyActor::CreateCube() {
	FVector cube_verts[8] = { {4,4,0}, {-4,4,0}, {4,-4,0}, {-4,-4,0}, {4,4,8}, {-4,4,8}, {4,-4,8}, {-4,-4,8} };
	int cube_tris[12][3] = { {3,2,0},{1,3,0}, {4,0,2},{4,2,6}, {4,5,1},{4,1,0}, {1,7,3},{5,7,1}, {2,7, 6 },{7, 2, 3}, {5, 4, 6}, {7, 5, 6} };
	TArray<FVector> vertices;
	for (int i = 0; i < 8; i++) {
		PointMass point;
		point.vert = (FVector)cube_verts[i];
		point.impulse = FVector(0, 0, 0);
		point.ind = i;
		point.vel = FVector(0, 0, 0);
		vertices.Add((FVector)cube_verts[i]);
		Points.Add(point);
	}
	/* one extra point mass for the center */
	FVector center = FVector(0, 0, 0);
	PointMass point;
	point.vert = center;
	point.impulse = FVector(0, 0, 0);
	point.ind = 12;
	point.vel = FVector(0, 0, 0);
	Points.Add(point);

	for (int i = 0; i < 12; i++) {
		int tri[3];
		for (int j = 0; j < 3; j++) {
			tri[j] = (cube_tris[i][j]);
			Triangles.Add(tri[j]);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("Some variable values: x: %f, y: %f z:%f"), vertices[0][0], vertices[0][1], vertices[0][2]));
		//Triangles.Add(tri);
	}
	TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	mesh->CreateMeshSection_LinearColor(1, vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), vertexColors, TArray<FProcMeshTangent>(), true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
	CreateSpringsCube();
}

void ASoftBodyActor::CreateSpringsCube() {
	int edges[12][2] = { {0,4},{1,5},{2,6},{3,7}, {0,1}, {1,3}, {3,2}, {2,0}, {4,5}, {5,7}, {7,6}, {6,4} };
	for (int i = 0; i < 12; i++) {
		PointMass* pm1, * pm2;
		pm1 = &(Points[edges[i][0]]);
		pm2 = &(Points[edges[i][1]]);
		Spring s;
		s.pm1 = pm1;
		s.pm2 = pm2;
		float edge_len = (pm1->vert - pm2->vert).Size();
		s.rest_len = s.cur_len = edge_len;
		Springs.Add(s);
	}

	/* internal springs */
	Spring s1, s2, s3, s4;
	s1.pm1 = &(Points[0]);
	s1.pm2 = &(Points[7]);
	s2.pm1 = &(Points[1]);
	s2.pm2 = &(Points[6]);
	s3.pm1 = &(Points[2]);
	s3.pm2 = &(Points[5]);
	s4.pm1 = &(Points[3]);
	s4.pm2 = &(Points[4]);
	float edge_len = (s1.pm1->vert - s1.pm2->vert).Size();
	s1.rest_len = s1.cur_len = edge_len;
	s2.rest_len = s2.cur_len = edge_len;
	s3.rest_len = s3.cur_len = edge_len;
	s4.rest_len = s4.cur_len = edge_len;
	Springs.Add(s1);
	Springs.Add(s2);
	Springs.Add(s3);
	Springs.Add(s4);
}

double ASoftBodyActor::GetSurfaceArea() {
	double SA = 0;
	for (int i = 0; i < Triangles.Num()/3; i++) {
		/*FVector v1, v2, v3;
		v1 = Points[Triangles[i * 3]].vert;
		v2 = Points[Triangles[i * 3 + 1]].vert;
		v3 = Points[Triangles[i * 3 + 2]].vert;
		double area = FVector().CrossProduct(v3 - v2, v3 - v1).Size() * 0.5;
		SA += area;*/
	}
	return SA;
}

/*called every timestep*/
void ASoftBodyActor::ResetImpulses() {
	for (int i = 0; i < Points.Num(); i++) {
		Points[i].impulse = FVector(0, 0, 0);
	}
}

void ASoftBodyActor::ProcessPointMasses(float DeltaTime) {
	/* calculate volume and surface area for internal gas force before updating any vertice locations*/
	double SA = GetSurfaceArea();
	FBoxSphereBounds bounds = mesh->GetLocalBounds();
	double Vol = bounds.GetSphere().GetVolume();
	for (int i = 0; i < Points.Num(); i++) {
		
		PointMass *p = &Points[i];
		/* Apply gravity.*/
		FVector delta = FVector(0,0,-GRAVITY*PM_MASS*DeltaTime);
		p->impulse += delta;
		//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("delta: x: %f, y: %f z:%f, "), delta.X, delta.Y, -9.8*DeltaTime*PM_MASS));
		FTransform t = mesh->GetRelativeTransform();
		/* Ground collision */
		if (t.TransformPosition(p->vert).Z < 100) {
			//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("locationn: x: %f, y: %f z:%f"), t.TransformPosition(p->vert).X, t.TransformPosition(p->vert).Y, t.TransformPosition(p->vert).Z));
			p->impulse += FVector(0, 0, GRAVITY * PM_MASS * DeltaTime);
			if (p->vel.Z < -0.1	) {
				p->impulse += FVector(0, 0, (p->vel.Z * PM_MASS * -1 * RESTITUTION));
				GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("AAHHHHimpulseeee: x: %f, y: %f z:%f colision impulse:%f, I:%d"), Points[0].impulse.X, Points[0].impulse.Y, Points[0].impulse.Z, t.TransformVector(p->vel).Z * PM_MASS * -1, i));

			}
			FVector z_loc = t.TransformPosition(p->vert);
			z_loc.Z = 100;
			//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("newwwwlocationn: x: %f, y: %f z:%f"), z_loc.X, z_loc.Y, z_loc.Z));
			z_loc =  t.InverseTransformPosition(z_loc);
			p->vert = z_loc;
			
		}

		/* internal gas force*/
		

		/*update velocities*/
		if (p->impulse.Size() != 0.0) {
			p->vel += p->impulse / PM_MASS;
			GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("velocity: x: %f, y: %f z:%f, I:%d"), Points[i].vel.X, Points[i].vel.Y, Points[i].vel.Z,i));
		}

		/* Euler integration for location*/
		p->vert += t.InverseTransformVector(p->vel) * (DeltaTime);
		GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("loca: x: %f, y: %f z:%f, I:%d"), Points[i].vert.X, Points[i].vert.Y, Points[i].vert.Z, i));

	}
	//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("impulseeee: x: %f, y: %f z:%f"), Points[0].impulse.X, Points[0].impulse.Y, Points[0].impulse.Z));

}

/* called before processing point masses. */
void ASoftBodyActor::ProcessSprings(float DeltaTime) {
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("NUMSPRINGS%d"),Springs.Num()));
	for (int i = 0; i < Springs.Num(); i++) {
		Spring* spring = &(Springs[i]);
		PointMass* pm1 = spring->pm1;
		PointMass* pm2 = spring->pm2;
		FTransform t = mesh->GetRelativeTransform();
		spring->cur_len = t.TransformVector(pm1->vert - pm2->vert).Size();
		
		FVector spring_dir = t.TransformVector((pm1->vert - pm2->vert));
		spring_dir.Normalize();
		float x = fabsf(spring->rest_len - spring->cur_len);
		float spring_impulse_mag = SPRING_K * x ;
		float damping_mag = SPRING_DAMP * (pm1->vel.Dot(spring_dir) - pm2->vel.Dot(spring_dir));
		spring_impulse_mag += damping_mag;
		if (spring->rest_len > spring->cur_len) {
			pm1->impulse += spring_dir * spring_impulse_mag;
			pm2->impulse += -spring_dir * spring_impulse_mag;
			//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("spring deets: sprinf impulse:%f %f %f"), (spring_dir * spring_impulse_mag).X, (spring_dir * spring_impulse_mag).Y, (spring_dir * spring_impulse_mag).Z));
		}
		else if(spring->rest_len < spring->cur_len) {
			pm1->impulse += -spring_dir * spring_impulse_mag;
			pm2->impulse += spring_dir * spring_impulse_mag;
			//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, FString::Printf(TEXT("spring deets: sprinf impulse:%f %f %f"), (spring_dir * spring_impulse_mag).X, (spring_dir * spring_impulse_mag).Y, (spring_dir * spring_impulse_mag).Z));
		}
	}
}

/*Update mesh. called after Processing pointmasses*/
void ASoftBodyActor::UpdateMesh() {
	TArray<FVector> vertices;
	for (int i = 0; i < Points.Num()-1; i++) {
		vertices.Add((FVector)Points[i].vert);
	}
	
	TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	mesh->UpdateMeshSection_LinearColor(1, vertices, TArray<FVector>(), TArray<FVector2D>(), vertexColors, TArray<FProcMeshTangent>());

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
}



