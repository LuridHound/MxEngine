#include <MxEngine.h>
using namespace MxEngine;

void InitGrid(MxObject& object)
{
	object.Transform.Scale(3.0f);

	object.Name = "Grid";
	object.AddComponent<MeshSource>(Primitives::CreatePlane(1000));

	auto material = object.GetOrAddComponent<MeshRenderer>()->GetMaterial();
	material->AlbedoMap = AssetManager::LoadTexture("Resources/textures/brick_albedo.jpg"_id);
	material->NormalMap = AssetManager::LoadTexture("Resources/textures/brick_normal.jpg"_id);
	material->AmbientOcclusionMap = AssetManager::LoadTexture("Resources/textures/brick_ao.jpg"_id);
	material->RoughnessMap = AssetManager::LoadTexture("Resources/textures/brick_roughness.jpg"_id);
}