// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and /or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Core/Resources/AssetManager.h"
#include "Core/Components/Lighting/LightBase.h"
#include <optional>

namespace MxEngine
{
	class Capsule;
	class Cylinder;
	class BoundingBox;
}

namespace MxEngine::GUI
{
	void DrawMeshEditor(const char* name, MeshHandle& mesh);
	void DrawAABBEditor(const char* name, AABB& aabb);
	void DrawBoxEditor(const char* name, BoundingBox& box);
	void DrawSphereEditor(const char* name, BoundingSphere& sphere);
	void DrawCylinderEditor(const char* name, Cylinder& cylinder);
	void DrawCapsuleEditor(const char* name, Capsule& capsule);
	void DrawLightBaseEditor(LightBase& base);
	void DrawVertexEditor(Vertex& vertex);
	void DrawImageSaver(const TextureHandle& texture, const char* name = "save texture to disk");
	void DrawTextureList(const char* name, bool* isOpen = nullptr);
}