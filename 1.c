#include <stdio.h>
#include <raylib.h>
#include <unistd.h>

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

Mesh makeMeshRectangle()
{
	Mesh mesh = {0};
	mesh.triangleCount = 2;
	mesh.vertexCount = 6;
	mesh.vertices = (float *)MemAlloc(sizeof(float) * 3 * 3 * 2);

	mesh.vertices[0] = 0.0f;
	mesh.vertices[1] = 10.0f;
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

int main()
{
	int windowWidth = 900;
	int windowHeight = 600;
	InitWindow(windowWidth, windowHeight, "Hello");

	SetTargetFPS(60);

	Camera camera = {{0.0f, 0.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 45.0f, 0};


//	Model myModelTriangle = LoadModelFromMesh(makeMeshTriangle());
//	Model myModelRectangle = LoadModelFromMesh(makeMeshRectangle());

	Mesh meshCube = GenMeshCube(1.0f, 1.0f, 1.0f);
//	meshCube.vertices[0] = 100.0f;
//	meshCube.vertices[3] = 100.0f;
//	meshCube.vertices[6] = 100.0f;
//	UploadMesh(&meshCube, false);

	Model modelCube = LoadModelFromMesh(meshCube);
	printf("model cube mesh count: %d\n", modelCube.meshCount);
	printf("model cube mesh 1 vertex count: %d\n", modelCube.meshes[0].vertexCount);
	printf("model cube mesh 1 triangle count: %d\n", modelCube.meshes[0].triangleCount);
	printf("model cube mesh 1 1st vertex: %f, %f, %f\n", modelCube.meshes[0].vertices[0], modelCube.meshes[0].vertices[1], modelCube.meshes[0].vertices[2]);
	printf("model cube mesh 1 2nd vertex: %f, %f, %f\n", modelCube.meshes[0].vertices[3], modelCube.meshes[0].vertices[4], modelCube.meshes[0].vertices[5]);


	Mesh meshSphere = GenMeshSphere(2.0f, 32, 32);
//	printf("mesh vertex count: %d\n", meshSphere.vertexCount);
//	printf("mesh triangle count: %d\n", meshSphere.triangleCount);

	Model modelSphere = LoadModelFromMesh(meshSphere);
//	printf("model material count: %d\n", modelSphere.materialCount);

//	Image image = GenImageChecked(2, 2, 1, 1, RED, BLUE);
	Image image = GenImageChecked(4, 4, 1, 1, RED, BLUE);
	Texture2D texture = LoadTextureFromImage(image);
	UnloadImage(image);

	modelSphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

//	sleep(10);
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_X) || IsKeyPressedRepeat(KEY_X)) {
			if (IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT)) {
				camera.position.x -= 0.1f;
			} else {
				camera.position.x += 0.1f;
			}
		}
		if (IsKeyPressed(KEY_Y) || IsKeyPressedRepeat(KEY_Y)) {
			if (IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT)) {
				camera.position.y -= 0.1f;
			} else {
				camera.position.y += 0.1f;
			}
		}
		if (IsKeyPressed(KEY_Z) || IsKeyPressedRepeat(KEY_Z)) {
			if (IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT)) {
				camera.position.z -= 0.1f;
			} else {
				camera.position.z += 0.1f;
			}
		}

		BeginDrawing();
			ClearBackground(BLACK);

			BeginMode3D(camera);
//				DrawModel(modelSphere, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
//				DrawModelWires(modelSphere, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);

////				DrawModel(modelCube, (Vector3){-4.0f, 0.0f, 0.0f}, 1.0f, WHITE);
				DrawModelWires(modelCube, (Vector3){-4.0f, 0.0f, 0.0f}, 1.0f, WHITE);
////				DrawModelPoints(modelCube, (Vector3){-4.0f, 0.0f, 0.0f}, 1.0f, WHITE);

//				DrawModel(myModelTriangle, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
//				DrawModel(myModelRectangle, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
//				DrawModelWires(myModelRectangle, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, BLUE);
			EndMode3D();

//			DrawTexture(texture, 10, 10, (Color){255, 255, 255, 255});
			DrawTextureEx(texture, (Vector2){10, 10}, 0.0f, 20.0f, (Color){255, 255, 255, 255});

			char text[1024];
			snprintf(text, sizeof(text), "x: %f\ny: %f\nz: %f", camera.position.x, camera.position.y, camera.position.z);
			DrawText(text, 100, 10, 20, WHITE);
		EndDrawing();
	}
//	CloseWindow();

	return 0;
}

