/*
RLAPI Vector2 GetMousePosition(void);                         // Get mouse position XY

RLAPI Ray GetScreenToWorldRay(Vector2 position, Camera camera)

// Ray, ray for raycasting
typedef struct Ray {
    Vector3 position;       // Ray position (origin)
    Vector3 direction;      // Ray direction (normalized)
} Ray;

RLAPI RayCollision GetRayCollisionTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3);            // Get collision info between ray and triangle

// RayCollision, ray hit information
typedef struct RayCollision {
    bool hit;               // Did the ray hit something?
    float distance;         // Distance to the nearest hit
    Vector3 point;          // Point of the nearest hit
    Vector3 normal;         // Surface normal of hit
} RayCollision;

https://antongerdelan.net/opengl/raycasting.html

RLAPI Vector2 GetWorldToScreen(Vector3 position, Camera camera);        // Get the screen space position for a 3d world space position

*/

//#define DEBUG_HIGHLIGHT_TRIANGLES

#include <stdio.h>
#include <raylib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <rlgl.h>
#include <stdlib.h>
#include <raymath.h>

//#include <limits.h>
#include <float.h>

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"


//#define MAX_VERTICES 1024 * 1024 * 16
#define MAX_VERTICES 1024 * 1024 * 32

int vertexCount = 0;
Vector3 positions[MAX_VERTICES]; //@ struct packing?
Vector2 positionsTexture[MAX_VERTICES]; //@ struct packing?
Vector3 normals[MAX_VERTICES]; //@ struct packing?

int uniqueVerticesCount = 0;
Vector3 uniqueVertices[MAX_VERTICES];
Vector3 accumulatedNormals[MAX_VERTICES];
int occurrenceCounts[MAX_VERTICES];

#ifdef DEBUG_HIGHLIGHT_TRIANGLES
// 2 highlighted triangles
#define TRIANGLES_MAX_VERTICES (12 * 3)
Vector3 trianglesVertices[TRIANGLES_MAX_VERTICES];
int trianglesIndexes[TRIANGLES_MAX_VERTICES];
int trianglesVertexCount = 0;
#endif

//#define GREEN_TRIANGLES_MAX_VERTICES (1024 * 3)
//Vector3 greenTrianglesVertices[TRIANGLES_MAX_VERTICES];
//int greenTrianglesVerticesCount = 0;

typedef struct VertexAttributes {
	Vector3 *position;
	Vector2 *positionTexture;
} VertexAttributes;

VertexAttributes appendVertex()
{
//	assert(vertexCount < MAX_VERTICES);
	if(vertexCount == MAX_VERTICES) {
		printf("%d\n", vertexCount);
		exit(1);
	}

	VertexAttributes result = {0};
	result.position = &positions[vertexCount];
	result.positionTexture = &positionsTexture[vertexCount];
	vertexCount += 1;
	return result;
}

Mesh makeMeshTriangle()
{
	Mesh mesh = {0};
	mesh.triangleCount = 1;
	mesh.vertexCount = 3;
	mesh.vertices = (float *)MemAlloc(sizeof(float) * 3 * 3);

//	mesh.vertices[0] = -1.0f;
//	mesh.vertices[1] = 0.0f;
//	mesh.vertices[2] = 0.0f;
//
//	mesh.vertices[3] = 0.0f;
//	mesh.vertices[4] = 1.0f;
//	mesh.vertices[5] = 0.0f;
//
//	mesh.vertices[6] = 1.0f;
//	mesh.vertices[7] = 0.0f;
//	mesh.vertices[8] = 0.0f;

	mesh.vertices[0] = -1.0f;
	mesh.vertices[1] = 0.0f;
	mesh.vertices[2] = 0.0f;

	mesh.vertices[3] = 1.0f;
	mesh.vertices[4] = 0.0f;
	mesh.vertices[5] = 0.0f;

	mesh.vertices[6] = 0.0f;
	mesh.vertices[7] = 1.0f;
	mesh.vertices[8] = 0.0f;

	UploadMesh(&mesh, false);

	return mesh;
}

Mesh makeDebugTriangle(Vector3 basePos, Vector3 tipPos)
//Model makeDebugTriangle(Vector3 basePos, Vector3 tipPos)
{
	Mesh mesh = {0};
	mesh.triangleCount = 2;
	mesh.vertexCount = 3 * mesh.triangleCount;
	mesh.vertices = (float *)MemAlloc(sizeof(float) * 3 * mesh.vertexCount);

	float baseLen = 0.1f;

//	float height = 1.0f;

	// left
	mesh.vertices[0] = basePos.x - (baseLen / 2.0f);
	mesh.vertices[1] = basePos.y;
	mesh.vertices[2] = basePos.z;

	// right
	mesh.vertices[3] = basePos.x + (baseLen / 2.0f);
	mesh.vertices[4] = basePos.y;
	mesh.vertices[5] = basePos.z;

	// upper
	mesh.vertices[6] = tipPos.x;
	mesh.vertices[7] = tipPos.y;
	mesh.vertices[8] = tipPos.z;

	// right
	mesh.vertices[9] = basePos.x + (baseLen / 2.0f);
	mesh.vertices[10] = basePos.y;
	mesh.vertices[11] = basePos.z;

	// left
	mesh.vertices[12] = basePos.x - (baseLen / 2.0f);
	mesh.vertices[13] = basePos.y;
	mesh.vertices[14] = basePos.z;

	// upper
	mesh.vertices[15] = tipPos.x;
	mesh.vertices[16] = tipPos.y;
	mesh.vertices[17] = tipPos.z;

	UploadMesh(&mesh, false);

	return mesh;
}

Mesh makeMeshRectangle()
{
	Mesh mesh = {0};
	mesh.triangleCount = 2;
	mesh.vertexCount = 6;
	mesh.vertices = (float *)MemAlloc(sizeof(float) * 3 * 3 * 2);

	mesh.vertices[0] = 0.0f;
	mesh.vertices[1] = 1.0f;
	mesh.vertices[2] = 0.0f;

	mesh.vertices[3] = 0.0f;
	mesh.vertices[4] = 0.0f;
	mesh.vertices[5] = 0.0f;

	mesh.vertices[6] = 1.0f;
	mesh.vertices[7] = 0.0f;
	mesh.vertices[8] = 0.0f;

	mesh.vertices[9] = 1.0f;
	mesh.vertices[10] = 0.0f;
	mesh.vertices[11] = 0.0f;

	mesh.vertices[12] = 1.0f;
	mesh.vertices[13] = 1.0f;
	mesh.vertices[14] = 0.0f;

	mesh.vertices[15] = 0.0f;
	mesh.vertices[16] = 1.0f;
	mesh.vertices[17] = 0.0f;

	UploadMesh(&mesh, false);

	return mesh;
}

//Mesh makeMeshPlane(Vector3 center, Vector3 planeX, Vector3 planeY, int width, int height)
//{
//	int numTriangles = 2 * width * height;
//	int numVertices = 3 * numTriangles;
//
//	Mesh mesh = {0};
//	mesh.triangleCount = numTriangles;
//	mesh.vertexCount = numVertices;
//	mesh.vertices = (float *)MemAlloc(sizeof(float) * 3 * numVertices);
//
//	Vector3 planeUpperLeft = center;
//
//	planeUpperLeft.x += -1.0f * planeX.x;
//	planeUpperLeft.y += -1.0f * planeX.y;
//	planeUpperLeft.z += -1.0f * planeX.z;
//
//	planeUpperLeft.x += 1.0f * planeY.x;
//	planeUpperLeft.y += 1.0f * planeY.y;
//	planeUpperLeft.z += 1.0f * planeY.z;
//
//	Vector3 upperLeft = planeUpperLeft;
////	printf("x: %f, y: %f, z: %f\n", upperLeft.x, upperLeft.y, upperLeft.z);
//
//	float smallWidth = 2.0f / width;
//	float smallHeight = 2.0f / height;
//
//	int baseIndex = 0;
//
//	for (int nthRow = 0; nthRow < height; ++nthRow) {
//		for (int nthColumn = 0; nthColumn < width; ++nthColumn) {
//			// upper left
//			mesh.vertices[baseIndex + 0] = upperLeft.x;
//			mesh.vertices[baseIndex + 1] = upperLeft.y;
//			mesh.vertices[baseIndex + 2] = upperLeft.z;
////			printf("%f, %f, %f\n", mesh.vertices[baseIndex + 0], mesh.vertices[baseIndex + 1], mesh.vertices[baseIndex + 2]);
//
//			// lower left
//			mesh.vertices[baseIndex + 3] = upperLeft.x - smallHeight * planeY.x;
//			mesh.vertices[baseIndex + 4] = upperLeft.y - smallHeight * planeY.y;
//			mesh.vertices[baseIndex + 5] = upperLeft.z - smallHeight * planeY.z;
//
//			// lower right
//			mesh.vertices[baseIndex + 6] = upperLeft.x - smallHeight * planeY.x + smallWidth * planeX.x;
//			mesh.vertices[baseIndex + 7] = upperLeft.y - smallHeight * planeY.y + smallWidth * planeX.y;
//			mesh.vertices[baseIndex + 8] = upperLeft.z - smallHeight * planeY.z + smallWidth * planeX.z;
//
//			// lower right
//			mesh.vertices[baseIndex + 9] = upperLeft.x - smallHeight * planeY.x + smallWidth * planeX.x;
//			mesh.vertices[baseIndex + 10] = upperLeft.y - smallHeight * planeY.y + smallWidth * planeX.y;
//			mesh.vertices[baseIndex + 11] = upperLeft.z - smallHeight * planeY.z + smallWidth * planeX.z;
//
//			// upper right
//			mesh.vertices[baseIndex + 12] = upperLeft.x + smallWidth * planeX.x;
//			mesh.vertices[baseIndex + 13] = upperLeft.y + smallWidth * planeX.y;
//			mesh.vertices[baseIndex + 14] = upperLeft.z + smallWidth * planeX.z;
//
//			// upper left
//			mesh.vertices[baseIndex + 15] = upperLeft.x;
//			mesh.vertices[baseIndex + 16] = upperLeft.y;
//			mesh.vertices[baseIndex + 17] = upperLeft.z;
//
//			baseIndex += 3 * 3 * 2;
//
//			upperLeft.x += smallWidth * planeX.x;
//			upperLeft.y += smallWidth * planeX.y;
//			upperLeft.z += smallWidth * planeX.z;
//		}
//
//		upperLeft.x -= width * smallWidth * planeX.x;
//		upperLeft.y -= width * smallWidth * planeX.y;
//		upperLeft.z -= width * smallWidth * planeX.z;
//
//		upperLeft.x -= smallHeight * planeY.x;
//		upperLeft.y -= smallHeight * planeY.y;
//		upperLeft.z -= smallHeight * planeY.z;
//	}
//
//	UploadMesh(&mesh, false);
//
//	return mesh;
//}

void makeVerticesPlane(Vector3 center, Vector3 planeX, Vector3 planeY, int width, int height)
{
	int numTriangles = 2 * width * height;
	int numVertices = 3 * numTriangles;

	Vector3 planeUpperLeft = center;

	planeUpperLeft.x += -1.0f * planeX.x;
	planeUpperLeft.y += -1.0f * planeX.y;
	planeUpperLeft.z += -1.0f * planeX.z;

	planeUpperLeft.x += 1.0f * planeY.x;
	planeUpperLeft.y += 1.0f * planeY.y;
	planeUpperLeft.z += 1.0f * planeY.z;

	Vector3 upperLeft = planeUpperLeft;
//	printf("x: %f, y: %f, z: %f\n", upperLeft.x, upperLeft.y, upperLeft.z);

	float smallWidth = 2.0f / width;
	float smallHeight = 2.0f / height;

	for (int nthRow = 0; nthRow < height; ++nthRow) {
		for (int nthColumn = 0; nthColumn < width; ++nthColumn) {
			VertexAttributes v;

			// upper left
			v = appendVertex();
			v.position->x = upperLeft.x;
			v.position->y = upperLeft.y;
			v.position->z = upperLeft.z;
//			v.positionTexture->x = (v.position->x + 1.0f) / 2.0f;
//			v.positionTexture->y = 1 - (v.position->y + 1.0f) / 2.0f;

			// lower left
			v = appendVertex();
			v.position->x = upperLeft.x - smallHeight * planeY.x;
			v.position->y = upperLeft.y - smallHeight * planeY.y;
			v.position->z = upperLeft.z - smallHeight * planeY.z;
//			v.positionTexture->x = (v.position->x + 1.0f) / 2.0f;
//			v.positionTexture->y = 1 - (v.position->y + 1.0f) / 2.0f;

			// lower right
			v = appendVertex();
			v.position->x = upperLeft.x - smallHeight * planeY.x + smallWidth * planeX.x;
			v.position->y = upperLeft.y - smallHeight * planeY.y + smallWidth * planeX.y;
			v.position->z = upperLeft.z - smallHeight * planeY.z + smallWidth * planeX.z;
//			v.positionTexture->x = (v.position->x + 1.0f) / 2.0f;
//			v.positionTexture->y = 1 - (v.position->y + 1.0f) / 2.0f;

			// lower right
			v = appendVertex();
			v.position->x = upperLeft.x - smallHeight * planeY.x + smallWidth * planeX.x;
			v.position->y = upperLeft.y - smallHeight * planeY.y + smallWidth * planeX.y;
			v.position->z = upperLeft.z - smallHeight * planeY.z + smallWidth * planeX.z;
//			v.positionTexture->x = (v.position->x + 1.0f) / 2.0f;
//			v.positionTexture->y = 1 - (v.position->y + 1.0f) / 2.0f;

			// upper right
			v = appendVertex();
			v.position->x = upperLeft.x + smallWidth * planeX.x;
			v.position->y = upperLeft.y + smallWidth * planeX.y;
			v.position->z = upperLeft.z + smallWidth * planeX.z;
//			v.positionTexture->x = (v.position->x + 1.0f) / 2.0f;
//			v.positionTexture->y = 1 - (v.position->y + 1.0f) / 2.0f;

			// upper left
			v = appendVertex();
			v.position->x = upperLeft.x;
			v.position->y = upperLeft.y;
			v.position->z = upperLeft.z;
//			v.positionTexture->x = (v.position->x + 1.0f) / 2.0f;
//			v.positionTexture->y = 1 - (v.position->y + 1.0f) / 2.0f;

			upperLeft.x += smallWidth * planeX.x;
			upperLeft.y += smallWidth * planeX.y;
			upperLeft.z += smallWidth * planeX.z;
		}

		upperLeft.x -= width * smallWidth * planeX.x;
		upperLeft.y -= width * smallWidth * planeX.y;
		upperLeft.z -= width * smallWidth * planeX.z;

		upperLeft.x -= smallHeight * planeY.x;
		upperLeft.y -= smallHeight * planeY.y;
		upperLeft.z -= smallHeight * planeY.z;
	}
}

void makeVerticesCube(void)
{
//	makeVerticesPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 700, 700); // right
//	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 700, 700); // front
//	makeVerticesPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 700, 700); // left
//	makeVerticesPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 700, 700); // back
//	makeVerticesPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 700, 700); // upper
//	makeVerticesPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 700, 700); // lower

//	makeVerticesPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 512, 512); // right
//	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 512, 512); // front
//	makeVerticesPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 512, 512); // left
//	makeVerticesPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 512, 512); // back
//	makeVerticesPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 512, 512); // upper
//	makeVerticesPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 512, 512); // lower

//	makeVerticesPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 256, 256); // right
//	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 256, 256); // front
//	makeVerticesPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 256, 256); // left
//	makeVerticesPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 256, 256); // back
//	makeVerticesPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 256, 256); // upper
//	makeVerticesPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 256, 256); // lower

//	makeVerticesPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 128, 128); // right
//	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 128, 128); // front
//	makeVerticesPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 128, 128); // left
//	makeVerticesPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 128, 128); // back
//	makeVerticesPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 128, 128); // upper
//	makeVerticesPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 128, 128); // lower

	makeVerticesPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 64, 64); // right
	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 64, 64); // front
	makeVerticesPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 64, 64); // left
	makeVerticesPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 64, 64); // back
	makeVerticesPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 64, 64); // upper
	makeVerticesPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 64, 64); // lower

//	makeVerticesPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 32, 32); // right
//	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 32, 32); // front
//	makeVerticesPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 32, 32); // left
//	makeVerticesPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 32, 32); // back
//	makeVerticesPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 32, 32); // upper
//	makeVerticesPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 32, 32); // lower

//	makeVerticesPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 16, 16); // right
//	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 16, 16); // front
//	makeVerticesPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 16, 16); // left
//	makeVerticesPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 16, 16); // back
//	makeVerticesPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 16, 16); // upper
//	makeVerticesPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 16, 16); // lower

//	makeVerticesPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 3, 3); // right
//	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 3, 3); // front
//	makeVerticesPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 3, 3); // left
//	makeVerticesPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 3, 3); // back
//	makeVerticesPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 3, 3); // upper
//	makeVerticesPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 3, 3); // lower

//	makeVerticesPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 1, 1); // right
//	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 1, 1); // front
//	makeVerticesPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 1, 1); // left
//	makeVerticesPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 1, 1); // back
//	makeVerticesPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 1, 1); // upper
//	makeVerticesPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 1, 1); // lower
}

void makeVerticesSphere(void)
{
	makeVerticesCube();

	for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
//		float length = sqrt(positions[vertexIndex].x * positions[vertexIndex].x + positions[vertexIndex].y * positions[vertexIndex].y + positions[vertexIndex].z * positions[vertexIndex].z);
		float length = sqrtf(positions[vertexIndex].x * positions[vertexIndex].x + positions[vertexIndex].y * positions[vertexIndex].y + positions[vertexIndex].z * positions[vertexIndex].z);
		positions[vertexIndex].x /= length;
		positions[vertexIndex].y /= length;
		positions[vertexIndex].z /= length;
	}
}

void makeVerticesIcosahedron(void)
{
//	VertexAttributes v;
//	float angle = 0.0f;
//
//	Vector3 curr;
//	Vector3 prev = (Vector3){cosf(angle), 0.0f, -sinf(angle)};
//	for (int nthSide = 0; nthSide < 5; ++nthSide) {
//		angle += 72.0f * 2.0f * 3.1415926 / 360.0f;
//		curr = (Vector3){cosf(angle), 0.0f, -sinf(angle)};
//		v = appendVertex(); *v.position = prev;
//		v = appendVertex(); *v.position = curr;
//		v = appendVertex(); *v.position = (Vector3){0.0f, 0.0f, 0.0f};
//		prev = curr;
//	}

	VertexAttributes v;
	float incr = 72.0f * 2.0f * 3.1415926 / 360.0f;
	float upper = 0.0f;
	float lower = upper + (incr / 2.0f);
	float vert_angle = atan(1.0f/2.0f);

	Vector3 up_tip = (Vector3){0.0f, 1.0f, 0.0f};
	Vector3 lo_tip = (Vector3){0.0f, -1.0f, 0.0f};
	Vector3 lo_curr;
	Vector3 lo_prev = (Vector3){cosf(vert_angle) * cosf(lower), sinf(-vert_angle), cosf(vert_angle) * -sinf(lower)};
	Vector3 up_curr;
	Vector3 up_prev = (Vector3){cosf(vert_angle) * cosf(upper), sinf(vert_angle), cosf(vert_angle) * -sinf(upper)};

	for (int nthSide = 0; nthSide < 5; ++nthSide) {
		upper += incr;
		lower += incr;

		up_curr = (Vector3){cosf(vert_angle) * cosf(upper), sinf(vert_angle), cosf(vert_angle) * -sinf(upper)};
		lo_curr = (Vector3){cosf(vert_angle) * cosf(lower), sinf(-vert_angle), cosf(vert_angle) * -sinf(lower)};

		// upper triangle
		v = appendVertex(); *v.position = up_tip;
		v = appendVertex(); *v.position = up_prev;
		v = appendVertex(); *v.position = up_curr;

		v = appendVertex(); *v.position = lo_prev;
		v = appendVertex(); *v.position = up_curr;
		v = appendVertex(); *v.position = up_prev;

		v = appendVertex(); *v.position = up_curr;
		v = appendVertex(); *v.position = lo_prev;
		v = appendVertex(); *v.position = lo_curr;

		// lower triangle
		v = appendVertex(); *v.position = lo_tip;
		v = appendVertex(); *v.position = lo_curr;
		v = appendVertex(); *v.position = lo_prev;

		up_prev = up_curr;
		lo_prev = lo_curr;
	}
}

float my_min(float a, float b) {
	return (a < b) ? a : b;
}

float my_max(float a, float b) {
	return (a > b) ? a : b;
}

float my_abs(float n) {
	return (n < 0.0f) ? -n : n;
}

int main()
{
//	float z, x;
//	z = 1.0f, x = 0.0f; printf("%f, %f -> %f\n", z, x, atan2(z, x));
//	z = 0.0f, x = -1.0f; printf("%f, %f -> %f\n", z, x, atan2(z, x));
//	z = -1.0f, x = 0.0f; printf("%f, %f -> %f\n", z, x, atan2(z, x));
//	z = 0.0f, x = 1.0f; printf("%f, %f -> %f\n", z, x, atan2(z, x));
//
//	z = 0.1f, x = -1.0f; printf("%f, %f -> %f\n", z, x, atan2(z, x));
//	z = -0.1f, x = -1.0f; printf("%f, %f -> %f\n", z, x, atan2(z, x));
//	return 0;

//	float myFloat = 1.2f;
//	float otherFloat = 1.31f;
//	printf("%.32f\n", myFloat);
//	printf("%.32f\n", otherFloat);
////	if(myFloat == otherFloat) {
////		printf("equal\n");
////	} else {
////		printf("not equal\n");
////	}
//	if(fabsf(myFloat - otherFloat) < 0.1f) {
//		printf("equal\n");
//	} else {
//		printf("not equal\n");
//	}
//	return 0;

//	printf("%lu\n", sizeof(vertices));

//	{
//	Vector3 *vertex = verticesAppend();
//	vertex->x = 1.0f;
//	vertex->y = 2.0f;
//	vertex->z = 3.0f;
//	}
//	{
//	Vector3 *vertex = verticesAppend();
//	vertex->x = 4.0f;
//	vertex->y = 5.0f;
//	vertex->z = 6.0f;
//	}
//	{
//	Vector3 *vertex = verticesAppend();
//	vertex->x = 3.141f;
//	vertex->y = 3.141f;
//	vertex->z = 3.141f;
//	}
//
//	for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
//		printf("%f, %f, %f\n", vertices[vertexIndex].x, vertices[vertexIndex].y, vertices[vertexIndex].z);
//	}
//
//	return 0;

	int windowWidth = 900;
	int windowHeight = 600;
	InitWindow(windowWidth, windowHeight, "Hello");

	SetTargetFPS(60);

	Vector3 cameraXAxis = {1.0f, 0.0f, 0.0f};
	Vector3 cameraYAxis = {0.0f, 1.0f, 0.0f};
	Vector3 cameraZAxis = {0.0f, 0.0f, -1.0f};

	Camera camera;
	{
		Vector3 cameraPosition = {0.0f, 0.0f, 3.0f};

		Vector3 cameraTarget;
		cameraTarget.x = cameraPosition.x + cameraZAxis.x;
		cameraTarget.y = cameraPosition.y + cameraZAxis.y;
		cameraTarget.z = cameraPosition.z + cameraZAxis.z;

		camera = (Camera){cameraPosition, cameraTarget, cameraYAxis, 45.0f, 0};
	}

//	Shader myShader = LoadShader("myShader.vs", "myShader.fs");
	Shader rlShader = LoadShader("lighting.vs", "lighting.fs");

	// Get some required shader locations
    rlShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(rlShader, "viewPos");
    // NOTE: "matModel" location name is automatically assigned on shader loading, 
    // no need to get the location again if using that uniform name
    //shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(rlShader, "ambient");
//    SetShaderValue(myShader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);
    SetShaderValue(rlShader, ambientLoc, (float[4]){ 3.0f, 3.0f, 3.0f, 3.0f }, SHADER_UNIFORM_VEC4);

//	Light light3 = CreateLight(LIGHT_DIRECTIONAL, (Vector3){10.0f, 0.0f, 0.0f}, Vector3Zero(), WHITE, myShader);
	Light light1 = CreateLight(LIGHT_POINT, (Vector3){50.0f, 2.0f, 0.0f}, Vector3Zero(), WHITE, rlShader);
	Light light2 = CreateLight(LIGHT_POINT, (Vector3){-10.0f, 1.0f, 0.0f}, Vector3Zero(), WHITE, rlShader);
//	Light light3 = CreateLight(LIGHT_POINT, (Vector3){0.0f, 3.0f, 10.0f}, Vector3Zero(), WHITE, myShader);
//	Light light4 = CreateLight(LIGHT_POINT, (Vector3){0.0f, 4.0f, -10.0f}, Vector3Zero(), WHITE, myShader);

////	Model myModelTriangle = LoadModelFromMesh(makeMeshTriangle());
////	Model myModelRectangle = LoadModelFromMesh(makeMeshRectangle());
//	Model modelRight = LoadModelFromMesh(makeMeshPlane((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, -1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 25, 25));
//	Model modelFront = LoadModelFromMesh(makeMeshPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 25, 25));
//	Model modelLeft = LoadModelFromMesh(makeMeshPlane((Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 25, 25));
//	Model modelBack = LoadModelFromMesh(makeMeshPlane((Vector3){0.0f, 0.0f, -1.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 25, 25));
//	Model modelUpper = LoadModelFromMesh(makeMeshPlane((Vector3){0.0f, 1.0f, 0.0f}, (Vector3){-1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 25, 25));
//	Model modelLower = LoadModelFromMesh(makeMeshPlane((Vector3){0.0f, -1.0f, 0.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 0.0f, 1.0f}, 25, 25));
//	printf("%d", vertexCount);
//	return 0;

//	makeVerticesPlane((Vector3){0.0f, 0.0f, 1.0f}, (Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, 1.0f, 0.0f}, 3, 2); // front
//	makeVerticesCube();
//	makeVerticesSphere();
	makeVerticesIcosahedron();

//	float min = FLT_MAX;
//	float max = FLT_MIN;
//	for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
////		positionsTexture[vertexIndex].x = -atan2(positions[vertexIndex].z, positions[vertexIndex].x) / (2.0f * 3.1415926);
//		float MY_PI = 3.1415926f;
//		positionsTexture[vertexIndex].x = -(atan2(positions[vertexIndex].z, positions[vertexIndex].x) + MY_PI) / (2.0f * MY_PI);
//		if(positionsTexture[vertexIndex].x < min) {min = positionsTexture[vertexIndex].x;}
//		if(positionsTexture[vertexIndex].x > max) {max = positionsTexture[vertexIndex].x;}
//
//		positionsTexture[vertexIndex].y = 1.0f - (positions[vertexIndex].y + 1.0f) / 2.0f;
//	}
//	printf("min = %f, max = %f\n", min, max);	
////	return 0;
//
//	/*
//	because the coordinate system "goes around"
//	0.0 and 1.0 and -1.0 refer to the same horizontal position on the texture
//	but when some vertices of the triangle land around 0.0 and others around 1.0, the interpolation between vertices (across the triangle) is not correct
//	-1.0 = 0.0, this means that -1.1 = -0.1
//	from -1.0 to -0.9: (-0.9) - (-1.0) = -0.9 + 1.0 = 0.1
//	so 2 vertices with the following x coordinates would be modified like this:
//	-0.2, -0.8 -> -0.2, 0.0 + -0.8 - (-1.0) = 0.2
//	*/
//	assert((vertexCount % 3) == 0);
//	int numTriangles = vertexCount / 3;
//	for (int triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex) {
//		int firstVertexIndex = triangleIndex * 3;
//		float x1 = positionsTexture[firstVertexIndex].x;
//		float x2 = positionsTexture[firstVertexIndex + 1].x;
//		float x3 = positionsTexture[firstVertexIndex + 2].x;
//
//		float new_x1 = x1;
//		float new_x2 = x2;
//		float new_x3 = x3;
//
//		float d11 = x1 - x2; d11 = my_abs(d11);
//		float d12 = x1 - x3; d12 = my_abs(d12);
//		float d13 = x2 - x3; d13 = my_abs(d13);
//
//		float d21 = my_abs(-1.0f - my_min(x1, x2)) + my_abs(my_max(x1, x2));
//		float d22 = my_abs(-1.0f - my_min(x1, x3)) + my_abs(my_max(x1, x3));
//		float d23 = my_abs(-1.0f - my_min(x2, x3)) + my_abs(my_max(x2, x3));
//
//		if (d11 >= d21) {if (x1 < x2) {new_x1 = x1 - (-1);} else {new_x2 = x2 - (-1);}}
//		if (d12 >= d22) {if (x1 < x3) {new_x1 = x1 - (-1);} else {new_x3 = x3 - (-1);}}
//		if (d13 >= d23) {if (x2 < x3) {new_x2 = x2 - (-1);} else {new_x3 = x3 - (-1);}}
//
//		positionsTexture[firstVertexIndex].x = new_x1;
//		positionsTexture[firstVertexIndex + 1].x = new_x2;
//		positionsTexture[firstVertexIndex + 2].x = new_x3;
//
////		if (d11 >= d21 || d12 >= d22 || d13 >= d23) {
//////			// modify
//////			assert(greenTrianglesVerticesCount < GREEN_TRIANGLES_MAX_VERTICES);
//////			greenTrianglesVertices[greenTrianglesVerticesCount] = positions[firstVertexIndex];
//////			greenTrianglesVerticesCount += 1;
//////
//////			assert(greenTrianglesVerticesCount < GREEN_TRIANGLES_MAX_VERTICES);
//////			greenTrianglesVertices[greenTrianglesVerticesCount] = positions[firstVertexIndex + 1];
//////			greenTrianglesVerticesCount += 1;
//////
//////			assert(greenTrianglesVerticesCount < GREEN_TRIANGLES_MAX_VERTICES);
//////			greenTrianglesVertices[greenTrianglesVerticesCount] = positions[firstVertexIndex + 2];
//////			greenTrianglesVerticesCount += 1;
////
////			//
////		} else {
////			// fine
////		}
//	}

//	Mesh greenTrianglesMesh = {0};
//	Model greenTrianglesModel = {0};
//	greenTrianglesMesh.triangleCount = greenTrianglesVerticesCount / 3;
//	greenTrianglesMesh.vertexCount = greenTrianglesVerticesCount;
//	greenTrianglesMesh.vertices = (float *)greenTrianglesVertices;
//	UploadMesh(&greenTrianglesMesh, false);
//
//	greenTrianglesModel = LoadModelFromMesh(greenTrianglesMesh);

//// displace vertices based on elevation info
//	const char *elevationMapPath = "/home/eero/raylib-game/3d/World_elevation_map.png";
//	Image elevationMapImage = LoadImage(elevationMapPath);
////	printf("%d x %d\n", elevationMapImage.width, elevationMapImage.height);
//	assert(elevationMapImage.format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
//
//	for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
//		int row = (int)((positions[vertexIndex].y + 1.0f) * (elevationMapImage.height - 1) / 2.0f);
//		row = elevationMapImage.height - 1 - row;
//		float angle = -atan2(positions[vertexIndex].z, positions[vertexIndex].x);
//		int column = (int)(angle * (elevationMapImage.width - 1.0f) / (2.0f * 3.1415926));
////		printf("row: %d\n", row);
//
//		unsigned char elevation = ((unsigned char *)elevationMapImage.data)[row * elevationMapImage.width + column];
////		printf("%d\n", elevation);
//
////		float multiplier = 1.0f + (elevation / 255.0f - 0.5f) * 0.1;
//////		float multiplier = 1.0f + (elevation / 255.0f - 0.5f) * 0.5;
////		positions[vertexIndex].x *= multiplier;
////		positions[vertexIndex].y *= multiplier;
////		positions[vertexIndex].z *= multiplier;
//
//		float displacement = (elevation / 255.0f - 0.5f) * 0.2f; // [-0.1, 0.1]
////		printf("%f\n", displacement);
//		float length = sqrt(positions[vertexIndex].x * positions[vertexIndex].x + positions[vertexIndex].y * positions[vertexIndex].y + positions[vertexIndex].z * positions[vertexIndex].z);
//		float unitX = positions[vertexIndex].x / length;
//		float unitY = positions[vertexIndex].y / length;
//		float unitZ = positions[vertexIndex].z / length;
//		float displacementX = unitX * displacement;
//		float displacementY = unitY * displacement;
//		float displacementZ = unitZ * displacement;
//
//		positions[vertexIndex].x += displacementX;
//		positions[vertexIndex].y += displacementY;
//		positions[vertexIndex].z += displacementZ;
//	}

	// calculate normals
	for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex += 3) {
		// a x b = |a| * |b| * sin(angle) * n
		// normal = normalize(a x b)

		Vector3 a = {0};
		a.x = positions[vertexIndex + 1].x - positions[vertexIndex].x;
		a.y = positions[vertexIndex + 1].y - positions[vertexIndex].y;
		a.z = positions[vertexIndex + 1].z - positions[vertexIndex].z;

		Vector3 b = {0};
		b.x = positions[vertexIndex + 2].x - positions[vertexIndex].x;
		b.y = positions[vertexIndex + 2].y - positions[vertexIndex].y;
		b.z = positions[vertexIndex + 2].z - positions[vertexIndex].z;

		Vector3 cross = {0};
		cross.x = a.y * b.z - a.z * b.y;
		cross.y = a.z * b.x - a.x * b.z;
		cross.z = a.x * b.y - a.y * b.x;

		// should normalize the cross product
		float length = sqrtf(cross.x * cross.x + cross.y * cross.y + cross.z * cross.z);
		cross.x /= length;
		cross.y /= length;
		cross.z /= length;

		normals[vertexIndex].x = cross.x;
		normals[vertexIndex].y = cross.y;
		normals[vertexIndex].z = cross.z;

		normals[vertexIndex + 1].x = cross.x;
		normals[vertexIndex + 1].y = cross.y;
		normals[vertexIndex + 1].z = cross.z;

		normals[vertexIndex + 2].x = cross.x;
		normals[vertexIndex + 2].y = cross.y;
		normals[vertexIndex + 2].z = cross.z;
	}


//	printf("vertices count: %d\n", vertexCount);
//	// vertex's new normal is the average of normals of triangles it's part of (?)
//	// indexInTriangle = vertexIndex % 3
//
//	for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
//		int found = 0;
//		for (int uniqueVerticesIndex = 0; uniqueVerticesIndex < uniqueVerticesCount; ++uniqueVerticesIndex) {
////			if (positions[vertexIndex].x == uniqueVertices[uniqueVerticesIndex].x && positions[vertexIndex].y == uniqueVertices[uniqueVerticesIndex].y && positions[vertexIndex].z == uniqueVertices[uniqueVerticesIndex].z)
//			if (fabsf(positions[vertexIndex].x - uniqueVertices[uniqueVerticesIndex].x) < 0.001f && fabsf(positions[vertexIndex].y - uniqueVertices[uniqueVerticesIndex].y) < 0.001f && fabsf(positions[vertexIndex].z - uniqueVertices[uniqueVerticesIndex].z) < 0.001f) {
//				occurrenceCounts[uniqueVerticesIndex] += 1;
//
//				accumulatedNormals[uniqueVerticesCount].x += normals[vertexIndex].x;
//				accumulatedNormals[uniqueVerticesCount].y += normals[vertexIndex].y;
//				accumulatedNormals[uniqueVerticesCount].z += normals[vertexIndex].z;
//
//				found = 1;
//				break;
//			}
//		}
//		printf("@ %d of %d\n", vertexIndex + 1, vertexCount);
//		if (found) {
//			continue;
//		} else {
//			assert(uniqueVerticesCount < MAX_VERTICES);
//
//			uniqueVertices[uniqueVerticesCount].x = positions[vertexIndex].x;
//			uniqueVertices[uniqueVerticesCount].y = positions[vertexIndex].y;
//			uniqueVertices[uniqueVerticesCount].z = positions[vertexIndex].z;
//
//			occurrenceCounts[uniqueVerticesCount] = 1;
//
//			accumulatedNormals[uniqueVerticesCount].x += normals[vertexIndex].x;
//			accumulatedNormals[uniqueVerticesCount].y += normals[vertexIndex].y;
//			accumulatedNormals[uniqueVerticesCount].z += normals[vertexIndex].z;
//
//			uniqueVerticesCount += 1;
//		}
//	}

	// cube has 8 corners. thats 8 corner vertices. half of which are part of 4 triangles and half are part of 5 triangles
	// rest are part of 6 triangles
	// so out of n vertices:
	// n - 8: 6 triangles
	// 4: 5 triangles
	// 4: 4 triangles
	// (unless I am missing something)
//	for (int uniqueVerticesIndex = 0; uniqueVerticesIndex < uniqueVerticesCount; ++uniqueVerticesIndex) {
//		printf("%f, %f, %f: %d\n", uniqueVertices[uniqueVerticesIndex].x, uniqueVertices[uniqueVerticesIndex].y, uniqueVertices[uniqueVerticesIndex].z, occurrenceCounts[uniqueVerticesIndex]);
//	}
//	printf("unique vertices count: %d\n", uniqueVerticesCount);

//	int sixes, fives, fours;
//	sixes = fives = fours = 0;
//	for (int uniqueVerticesIndex = 0; uniqueVerticesIndex < uniqueVerticesCount; ++uniqueVerticesIndex) {
//		assert(occurrenceCounts[uniqueVerticesIndex] == 6 || occurrenceCounts[uniqueVerticesIndex] == 5 || occurrenceCounts[uniqueVerticesIndex] == 4);
//
//		if (occurrenceCounts[uniqueVerticesIndex] == 6) {sixes += 1;}
//		else if (occurrenceCounts[uniqueVerticesIndex] == 5) {fives += 1;}
//		else {fours += 1;}
//	}
//	assert(sixes == ((vertexCount - 4 * 5 - 4 * 4) / 6));
//	assert(fives == 4);
//	assert(fours == 4);
//
//	for (int uniqueVerticesIndex = 0; uniqueVerticesIndex < uniqueVerticesCount; ++uniqueVerticesIndex) {
////		accumulatedNormals[uniqueVerticesIndex].x /= occurrenceCounts[uniqueVerticesIndex];
////		accumulatedNormals[uniqueVerticesIndex].y /= occurrenceCounts[uniqueVerticesIndex];
////		accumulatedNormals[uniqueVerticesIndex].z /= occurrenceCounts[uniqueVerticesIndex];
//
//		float length = sqrt(accumulatedNormals[uniqueVerticesIndex].x * accumulatedNormals[uniqueVerticesIndex].x + accumulatedNormals[uniqueVerticesIndex].y * accumulatedNormals[uniqueVerticesIndex].y + accumulatedNormals[uniqueVerticesIndex].z * accumulatedNormals[uniqueVerticesIndex].z);
//		accumulatedNormals[uniqueVerticesIndex].x /= length;
//		accumulatedNormals[uniqueVerticesIndex].y /= length;
//		accumulatedNormals[uniqueVerticesIndex].z /= length;
//	}
//
//	for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
//		int found = 0;
//		for (int uniqueVerticesIndex = 0; uniqueVerticesIndex < uniqueVerticesCount; ++uniqueVerticesIndex) {
////			if (positions[vertexIndex].x == uniqueVertices[uniqueVerticesIndex].x && positions[vertexIndex].y == uniqueVertices[uniqueVerticesIndex].y && positions[vertexIndex].z == uniqueVertices[uniqueVerticesIndex].z)
//			if (fabsf(positions[vertexIndex].x - uniqueVertices[uniqueVerticesIndex].x) < 0.001f && fabsf(positions[vertexIndex].y - uniqueVertices[uniqueVerticesIndex].y) < 0.001f && fabsf(positions[vertexIndex].z - uniqueVertices[uniqueVerticesIndex].z) < 0.001f) {
//				normals[vertexIndex].x = accumulatedNormals[uniqueVerticesIndex].x;
//				normals[vertexIndex].y = accumulatedNormals[uniqueVerticesIndex].y;
//				normals[vertexIndex].z = accumulatedNormals[uniqueVerticesIndex].z;
//
//				found = 1;
//				break;
//			}
//		}
//		assert(found);
//	}

	assert((vertexCount % 3) == 0);
	int triangleCount = vertexCount / 3;
	Mesh earthMesh = {0};
	earthMesh.vertexCount = vertexCount;
	earthMesh.triangleCount = triangleCount;
	earthMesh.vertices = (float *)positions;
//	earthMesh.texcoords = (float *)positionsTexture;
	earthMesh.normals = (float *)normals;

	UploadMesh(&earthMesh, false);

	Model earthModel = LoadModelFromMesh(earthMesh);
	earthModel.materials[0].shader = rlShader;
//	earthModel.materials[0].shader = myShader;


//	vertexCount = 0;
//
//	makeVerticesSphere();
//
//	assert((vertexCount % 3) == 0);
//	Mesh myMesh2 = {0};
//	myMesh2.vertexCount = vertexCount;
//	myMesh2.triangleCount = vertexCount / 3;
//	myMesh2.vertices = (float *)positions;
////	myMesh2.texcoords = (float *)positionsTexture;
//
//	UploadMesh(&myMesh2, false);
//
//	Model myModel2 = LoadModelFromMesh(myMesh2);

//	make debug triangles
	Model debugTriangleModels[3];
	for(int vertexIndex = 0; vertexIndex < 3; ++vertexIndex) {
		Vector3 basePos = positions[vertexIndex];
		Vector3 tipPos = {basePos.x + 0.0f, basePos.y + 0.1f, basePos.z + 0.0f};
		debugTriangleModels[vertexIndex] = LoadModelFromMesh(makeDebugTriangle(basePos, tipPos));
	}

//	for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
//		printf("%f, %f, %f\n", normals[vertexIndex].x, normals[vertexIndex].y, normals[vertexIndex].z);
//	}
////	return 0;
//
//	#define NUM_DEBUG_TRIANGLES 1024
////	Model triangles[1024 * 10];
//	Model triangles[1024];
////	for (int triangleIndex = 0; triangleIndex < NUM_DEBUG_TRIANGLES; ++triangleIndex) {
//	for (int triangleIndex = 0; triangleIndex < vertexCount; ++triangleIndex) {
//		Vector3 basePos = positions[triangleIndex];
//		Vector3 tipPos = {basePos.x + normals[triangleIndex].x, basePos.y + normals[triangleIndex].y, basePos.z + normals[triangleIndex].z};
//		Mesh triangleMesh = makePointyTriangle2(basePos, tipPos);
//
//		triangles[triangleIndex] = LoadModelFromMesh(triangleMesh);
//	}

//	Texture2D textureMap = LoadTexture("/home/eero/raylib-game/3d/Large_World_Topo_Map_2.png");
//	Texture2D textureMap = LoadTexture("/home/eero/raylib-game/3d/Large_World_Topo_Map_2(upside-down).png");
//	Texture2D textureMap = LoadTexture("/home/eero/raylib-game/3d/2560px-Large_World_Topo_Map_2.png");
//	Texture2D textureMap = LoadTexture("/home/eero/raylib-game/3d/LargestWorldMap.png");
////	Texture2D textureMap = LoadTexture("/home/eero/raylib-game/3d/world.200412.3x5400x2700.png");
	Texture2D textureMap = LoadTexture("/home/eero/raylib-game/3d/world.200407.3x5400x2700.png");
//	Texture2D textureMap = LoadTexture("/home/eero/raylib-game/3d/land_ocean_ice_8192.png");
//	Texture2D textureMap = LoadTexture("/home/eero/raylib-game/3d/world.200401.3x5400x2700.png");
//	Texture2D textureMap = LoadTexture("/home/eero/raylib-game/3d/world.200411.3x21600x10800.jpg");

//	int worldMapLoc = GetShaderLocation(myShader, "worldMap");
	int worldMapLoc = GetShaderLocation(rlShader, "worldMap");
//    rlEnableShader(myShader.id);
    rlEnableShader(rlShader.id);
    rlActiveTextureSlot(1);
    rlEnableTexture(textureMap.id);
    rlSetUniformSampler(worldMapLoc, textureMap.id);
//    rlSetUniformSampler(worldMapLoc, 1);

	while (!WindowShouldClose()) {
#ifdef DEBUG_HIGHLIGHT_TRIANGLES
		Vector2 mouse_pos = GetMousePosition();
//		printf("mouse x: %f, mouse y: %f\n", mouse_pos.x, mouse_pos.y);

		Ray ray = GetScreenToWorldRay(mouse_pos, camera);

//		printf("vertexCount: %d\n", vertexCount);
//		RayCollision collision = GetRayCollisionSphere(ray, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f);
//		printf("%d\n", collision.hit);
		int hit_count = 0;
		Vector3 hit_points[TRIANGLES_MAX_VERTICES / 3] = {0};
		for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex += 3) {
			RayCollision collision = GetRayCollisionTriangle(ray, positions[vertexIndex], positions[vertexIndex + 1], positions[vertexIndex + 2]);
			if (collision.hit) {
				hit_count += 1;

				hit_points[trianglesVertexCount / 3] = collision.point;

				assert(trianglesVertexCount < TRIANGLES_MAX_VERTICES);
				trianglesVertices[trianglesVertexCount] = positions[vertexIndex];
				trianglesIndexes[trianglesVertexCount] = vertexIndex;
				trianglesVertexCount += 1;

				assert(trianglesVertexCount < TRIANGLES_MAX_VERTICES);
				trianglesVertices[trianglesVertexCount] = positions[vertexIndex + 1];
				trianglesIndexes[trianglesVertexCount] = vertexIndex + 1;
				trianglesVertexCount += 1;

				assert(trianglesVertexCount < TRIANGLES_MAX_VERTICES);
				trianglesVertices[trianglesVertexCount] = positions[vertexIndex + 2];
				trianglesIndexes[trianglesVertexCount] = vertexIndex + 2;
				trianglesVertexCount += 1;
			}
		}
		printf("trianglesVertexCount: %d\n", trianglesVertexCount);
		printf("triangle hit count: %d\n", hit_count);

		Mesh trianglesMesh = {0};
		Model trianglesModel = {0};

		if (hit_count > 0) {
//			if (hit_count != 2) {
//				for (int trianglesVerticesIndex = 0; trianglesVerticesIndex < trianglesVertexCount; trianglesVerticesIndex += 3) {
//					printf("hit point: (%f, %f, %f)", hit_points[trianglesVerticesIndex / 3].x, trianglesVertices[trianglesVerticesIndex / 3].y, trianglesVertices[trianglesVerticesIndex / 3].z);
//					printf(" (%f, %f, %f)", trianglesVertices[trianglesVerticesIndex].x, trianglesVertices[trianglesVerticesIndex].y, trianglesVertices[trianglesVerticesIndex].z);
//					printf(" (%f, %f, %f)", trianglesVertices[trianglesVerticesIndex + 1].x, trianglesVertices[trianglesVerticesIndex + 1].y, trianglesVertices[trianglesVerticesIndex + 1].z);
//					printf(" (%f, %f, %f)", trianglesVertices[trianglesVerticesIndex + 2].x, trianglesVertices[trianglesVerticesIndex + 2].y, trianglesVertices[trianglesVerticesIndex + 2].z);
//					printf("\n");
//				}
//				exit(0);
//			}

			trianglesMesh.triangleCount = hit_count;
			trianglesMesh.vertexCount = trianglesMesh.triangleCount * 3;
			trianglesMesh.vertices = (float *)trianglesVertices;
			UploadMesh(&trianglesMesh, false);

			trianglesModel = LoadModelFromMesh(trianglesMesh);
		}
#endif

//		if (IsKeyPressed(KEY_X) || IsKeyPressedRepeat(KEY_X)) {
//			if (IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT)) {
//				camera.position.x -= 0.1f;
//			} else {
//				camera.position.x += 0.1f;
//			}
//		}
//		if (IsKeyPressed(KEY_Y) || IsKeyPressedRepeat(KEY_Y)) {
//			if (IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT)) {
//				camera.position.y -= 0.1f;
//			} else {
//				camera.position.y += 0.1f;
//			}
//		}
//		if (IsKeyPressed(KEY_Z) || IsKeyPressedRepeat(KEY_Z)) {
//			if (IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT)) {
//				camera.position.z -= 0.1f;
//			} else {
//				camera.position.z += 0.1f;
//			}
//		}

		if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
//			camera.position.z -= 0.1f;

			camera.position.x += 0.1f * cameraZAxis.x;
			camera.position.y += 0.1f * cameraZAxis.y;
			camera.position.z += 0.1f * cameraZAxis.z;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
//			camera.position.z += 0.1f;

			camera.position.x -= 0.1f * cameraZAxis.x;
			camera.position.y -= 0.1f * cameraZAxis.y;
			camera.position.z -= 0.1f * cameraZAxis.z;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}
		if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
//			camera.position.x -= 0.1f;

			camera.position.x -= 0.1f * cameraXAxis.x;
			camera.position.y -= 0.1f * cameraXAxis.y;
			camera.position.z -= 0.1f * cameraXAxis.z;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}
		if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
//			camera.position.x += 0.1f;

			camera.position.x += 0.1f * cameraXAxis.x;
			camera.position.y += 0.1f * cameraXAxis.y;
			camera.position.z += 0.1f * cameraXAxis.z;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}
		if (IsKeyPressed(KEY_PAGE_UP) || IsKeyPressedRepeat(KEY_PAGE_UP)) {
//			camera.position.y += 0.1f;

			camera.position.x += 0.1f * cameraYAxis.x;
			camera.position.y += 0.1f * cameraYAxis.y;
			camera.position.z += 0.1f * cameraYAxis.z;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}
		if (IsKeyPressed(KEY_PAGE_DOWN) || IsKeyPressedRepeat(KEY_PAGE_DOWN)) {
//			camera.position.y -= 0.1f;

			camera.position.x -= 0.1f * cameraYAxis.x;
			camera.position.y -= 0.1f * cameraYAxis.y;
			camera.position.z -= 0.1f * cameraYAxis.z;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
		}

		if (IsKeyPressed(KEY_A) || IsKeyPressedRepeat(KEY_A)) {
			float deltaAngle = -0.1f;

			Vector3 cameraNewXAxis;
			cameraNewXAxis.x = cos(deltaAngle) * cameraXAxis.x - sin(deltaAngle) * cameraZAxis.x;
			cameraNewXAxis.y = cos(deltaAngle) * cameraXAxis.y - sin(deltaAngle) * cameraZAxis.y;
			cameraNewXAxis.z = cos(deltaAngle) * cameraXAxis.z - sin(deltaAngle) * cameraZAxis.z;
//			v3f cameraNewXAxis = cos(deltaAngle) * cameraXAxis - sin(deltaAngle) * cameraZAxis;
//			v3f cameraNewXAxis = v3f_sub(v3f_mul(cos(deltaAngle), cameraXAxis), v3f_mul(sin(deltaAngle) * cameraZAxis));

			Vector3 cameraNewYAxis;
			cameraNewYAxis.x = cameraYAxis.x;
			cameraNewYAxis.y = cameraYAxis.y;
			cameraNewYAxis.z = cameraYAxis.z;

			Vector3 cameraNewZAxis;
			cameraNewZAxis.x = cos(deltaAngle) * cameraZAxis.x + sin(deltaAngle) * cameraXAxis.x;
			cameraNewZAxis.y = cos(deltaAngle) * cameraZAxis.y + sin(deltaAngle) * cameraXAxis.y;
			cameraNewZAxis.z = cos(deltaAngle) * cameraZAxis.z + sin(deltaAngle) * cameraXAxis.z;

			cameraXAxis = cameraNewXAxis;
			cameraYAxis = cameraNewYAxis;
			cameraZAxis = cameraNewZAxis;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}
		if (IsKeyPressed(KEY_D) || IsKeyPressedRepeat(KEY_D)) {
			float deltaAngle = 0.1f;

			Vector3 cameraNewXAxis;
			cameraNewXAxis.x = cos(deltaAngle) * cameraXAxis.x - sin(deltaAngle) * cameraZAxis.x;
			cameraNewXAxis.y = cos(deltaAngle) * cameraXAxis.y - sin(deltaAngle) * cameraZAxis.y;
			cameraNewXAxis.z = cos(deltaAngle) * cameraXAxis.z - sin(deltaAngle) * cameraZAxis.z;
//			v3f cameraNewXAxis = cos(deltaAngle) * cameraXAxis - sin(deltaAngle) * cameraZAxis;
//			v3f cameraNewXAxis = v3f_sub(v3f_mul(cos(deltaAngle), cameraXAxis), v3f_mul(sin(deltaAngle) * cameraZAxis));

			Vector3 cameraNewYAxis;
			cameraNewYAxis.x = cameraYAxis.x;
			cameraNewYAxis.y = cameraYAxis.y;
			cameraNewYAxis.z = cameraYAxis.z;

			Vector3 cameraNewZAxis;
			cameraNewZAxis.x = cos(deltaAngle) * cameraZAxis.x + sin(deltaAngle) * cameraXAxis.x;
			cameraNewZAxis.y = cos(deltaAngle) * cameraZAxis.y + sin(deltaAngle) * cameraXAxis.y;
			cameraNewZAxis.z = cos(deltaAngle) * cameraZAxis.z + sin(deltaAngle) * cameraXAxis.z;

			cameraXAxis = cameraNewXAxis;
			cameraYAxis = cameraNewYAxis;
			cameraZAxis = cameraNewZAxis;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}
		if (IsKeyPressed(KEY_W) || IsKeyPressedRepeat(KEY_W)) {
			float deltaAngle = 0.1f;

			Vector3 cameraNewXAxis;
			cameraNewXAxis.x = cameraXAxis.x;
			cameraNewXAxis.y = cameraXAxis.y;
			cameraNewXAxis.z = cameraXAxis.z;

			Vector3 cameraNewYAxis;
			cameraNewYAxis.x = cos(deltaAngle) * cameraYAxis.x - sin(deltaAngle) * cameraZAxis.x;
			cameraNewYAxis.y = cos(deltaAngle) * cameraYAxis.y - sin(deltaAngle) * cameraZAxis.y;
			cameraNewYAxis.z = cos(deltaAngle) * cameraYAxis.z - sin(deltaAngle) * cameraZAxis.z;

			Vector3 cameraNewZAxis;
			cameraNewZAxis.x = cos(deltaAngle) * cameraZAxis.x + sin(deltaAngle) * cameraYAxis.x;
			cameraNewZAxis.y = cos(deltaAngle) * cameraZAxis.y + sin(deltaAngle) * cameraYAxis.y;
			cameraNewZAxis.z = cos(deltaAngle) * cameraZAxis.z + sin(deltaAngle) * cameraYAxis.z;

			cameraXAxis = cameraNewXAxis;
			cameraYAxis = cameraNewYAxis;
			cameraZAxis = cameraNewZAxis;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}
		if (IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_S)) {
			float deltaAngle = -0.1f;

			Vector3 cameraNewXAxis;
			cameraNewXAxis.x = cameraXAxis.x;
			cameraNewXAxis.y = cameraXAxis.y;
			cameraNewXAxis.z = cameraXAxis.z;

			Vector3 cameraNewYAxis;
			cameraNewYAxis.x = cos(deltaAngle) * cameraYAxis.x - sin(deltaAngle) * cameraZAxis.x;
			cameraNewYAxis.y = cos(deltaAngle) * cameraYAxis.y - sin(deltaAngle) * cameraZAxis.y;
			cameraNewYAxis.z = cos(deltaAngle) * cameraYAxis.z - sin(deltaAngle) * cameraZAxis.z;

			Vector3 cameraNewZAxis;
			cameraNewZAxis.x = cos(deltaAngle) * cameraZAxis.x + sin(deltaAngle) * cameraYAxis.x;
			cameraNewZAxis.y = cos(deltaAngle) * cameraZAxis.y + sin(deltaAngle) * cameraYAxis.y;
			cameraNewZAxis.z = cos(deltaAngle) * cameraZAxis.z + sin(deltaAngle) * cameraYAxis.z;

			cameraXAxis = cameraNewXAxis;
			cameraYAxis = cameraNewYAxis;
			cameraZAxis = cameraNewZAxis;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;

		}
		if (IsKeyPressed(KEY_E) || IsKeyPressedRepeat(KEY_E)) {
			float deltaAngle = 0.1f;

			Vector3 cameraNewZAxis;
			cameraNewZAxis.x = cameraZAxis.x;
			cameraNewZAxis.y = cameraZAxis.y;
			cameraNewZAxis.z = cameraZAxis.z;

			Vector3 cameraNewYAxis;
			cameraNewYAxis.x = cos(deltaAngle) * cameraYAxis.x - sin(deltaAngle) * cameraXAxis.x;
			cameraNewYAxis.y = cos(deltaAngle) * cameraYAxis.y - sin(deltaAngle) * cameraXAxis.y;
			cameraNewYAxis.z = cos(deltaAngle) * cameraYAxis.z - sin(deltaAngle) * cameraXAxis.z;

			Vector3 cameraNewXAxis;
			cameraNewXAxis.x = cos(deltaAngle) * cameraXAxis.x + sin(deltaAngle) * cameraYAxis.x;
			cameraNewXAxis.y = cos(deltaAngle) * cameraXAxis.y + sin(deltaAngle) * cameraYAxis.y;
			cameraNewXAxis.z = cos(deltaAngle) * cameraXAxis.z + sin(deltaAngle) * cameraYAxis.z;

			cameraXAxis = cameraNewXAxis;
			cameraYAxis = cameraNewYAxis;
			cameraZAxis = cameraNewZAxis;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}
		if (IsKeyPressed(KEY_Q) || IsKeyPressedRepeat(KEY_Q)) {
			float deltaAngle = -0.1;

			Vector3 cameraNewZAxis;
			cameraNewZAxis.x = cameraZAxis.x;
			cameraNewZAxis.y = cameraZAxis.y;
			cameraNewZAxis.z = cameraZAxis.z;

			Vector3 cameraNewYAxis;
			cameraNewYAxis.x = cos(deltaAngle) * cameraYAxis.x - sin(deltaAngle) * cameraXAxis.x;
			cameraNewYAxis.y = cos(deltaAngle) * cameraYAxis.y - sin(deltaAngle) * cameraXAxis.y;
			cameraNewYAxis.z = cos(deltaAngle) * cameraYAxis.z - sin(deltaAngle) * cameraXAxis.z;

			Vector3 cameraNewXAxis;
			cameraNewXAxis.x = cos(deltaAngle) * cameraXAxis.x + sin(deltaAngle) * cameraYAxis.x;
			cameraNewXAxis.y = cos(deltaAngle) * cameraXAxis.y + sin(deltaAngle) * cameraYAxis.y;
			cameraNewXAxis.z = cos(deltaAngle) * cameraXAxis.z + sin(deltaAngle) * cameraYAxis.z;

			cameraXAxis = cameraNewXAxis;
			cameraYAxis = cameraNewYAxis;
			cameraZAxis = cameraNewZAxis;

			Vector3 cameraTarget;
			cameraTarget.x = camera.position.x + cameraZAxis.x;
			cameraTarget.y = camera.position.y + cameraZAxis.y;
			cameraTarget.z = camera.position.z + cameraZAxis.z;

			camera.target = cameraTarget;
			camera.up = cameraYAxis;
		}

		// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
//        SetShaderValue(myShader, myShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        SetShaderValue(rlShader, rlShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

//		UpdateLightValues(myShader, light);

		BeginDrawing();
			ClearBackground(BLACK);

			BeginMode3D(camera);
//				DrawGrid(3.0f, 1.0f);
//				DrawModel(myModelTriangle, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
//				DrawModel(myModelRectangle, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
//				DrawModelWires(myModelTriangle, (Vector3){2.0f, 0.0f, 0.0f}, 1.0f, RED);
//				DrawModelWires(myModelRectangle, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);

//				DrawModelWires(modelRight, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
//				DrawModelWires(modelFront, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
//				DrawModelWires(modelLeft, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
//				DrawModelWires(modelBack, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
//				DrawModelWires(modelUpper, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
//				DrawModelWires(modelLower, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);

//				DrawModelWires(myModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);

//				BeginShaderMode(myShader);
				BeginShaderMode(rlShader);
//					SetShaderValueTexture(myShader, worldMapLoc, textureMap);
//					DrawModel(earthModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, PURPLE);
					DrawModelWires(earthModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
//					DrawModel(myModel2, (Vector3){0.0f, 0.0f, 0.0f}, 1.0121f, (Color){0, 30, 100, 255});
//					DrawModelWires(myModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, PURPLE);
//					DrawModelWires(myModel2, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, PURPLE);
				EndShaderMode();

				for(int triangleIndex = 0; triangleIndex < 3; ++triangleIndex) {
					DrawModel(debugTriangleModels[triangleIndex], (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, RED);
				}
//					for (int triangleIndex = 0; triangleIndex < NUM_DEBUG_TRIANGLES; ++triangleIndex) {
//					for (int triangleIndex = 0; triangleIndex < vertexCount; ++triangleIndex) {
//						DrawModel(triangles[triangleIndex], (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, RED);
//					}

#ifdef DEBUG_HIGHLIGHT_TRIANGLES
				if (hit_count > 0) {
					DrawModel(trianglesModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, RED);
				}
#endif

//				DrawModel(greenTrianglesModel, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, GREEN);
			EndMode3D();

//			DrawTextureEx(textureMap, (Vector2){100.0f, 100.0f}, 0.0f, 0.25f, (Color){255, 255, 255, 255});

#ifdef DEBUG_HIGHLIGHT_TRIANGLES
			for (int trianglesVerticesIndex = 0; trianglesVerticesIndex < trianglesVertexCount; ++trianglesVerticesIndex) {
				Vector2 vertexScreenPos = GetWorldToScreen(trianglesVertices[trianglesVerticesIndex], camera);

//				int vertexIndex = trianglesIndexes[trianglesVerticesIndex];
//				Vector2 texCoord = positionsTexture[vertexIndex];
//				char text[64]; snprintf(text, sizeof(text), "%f, %f", texCoord.x, texCoord.y);
////				char text[64]; snprintf(text, sizeof(text), "%.2f, %.2f", texCoord.x, texCoord.y);

				char text[64]; snprintf(text, sizeof(text), "%.2f, %.2f, %.2f", trianglesVertices[trianglesVerticesIndex].x, trianglesVertices[trianglesVerticesIndex].y, trianglesVertices[trianglesVerticesIndex].z);
				DrawText(text, vertexScreenPos.x, vertexScreenPos.y, 16, WHITE);
			}
#endif

			char text[1024];
			snprintf(text, sizeof(text), "x: %f\ny: %f\nz: %f", camera.position.x, camera.position.y, camera.position.z);
			DrawText(text, 100, 10, 20, WHITE);
		EndDrawing();

//		UnloadModel(trianglesModel);

#ifdef DEBUG_HIGHLIGHT_TRIANGLES
		trianglesVertexCount = 0;
#endif		
	}

	UnloadShader(rlShader);
//	UnloadShader(myShader);
//	CloseWindow();

	return 0;
}

