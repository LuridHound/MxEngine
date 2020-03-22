// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditionsand the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditionsand the following disclaimer in the documentation
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

#include "Core/Application/Application.h"

namespace MxEngine::GUI
{
    inline void DrawLightEditor()
    {
        auto context = Application::Get();

		auto& scene = context->GetCurrentScene();
		auto& globalLight = scene.GlobalLight;

		if (ImGui::CollapsingHeader("global light", ImGuiTreeNodeFlags_None))
		{
			ImGui::PushID(0);

			auto ambient = globalLight.GetAmbientColor();
			auto diffuse = globalLight.GetDiffuseColor();
			auto specular = globalLight.GetSpecularColor();
			ImGui::InputFloat3("direction", &globalLight.Direction[0]);

			if (ImGui::InputFloat3("ambient color", &ambient[0]))
				globalLight.UseAmbientColor(ambient);
			if (ImGui::InputFloat3("diffuse color", &diffuse[0]))
				globalLight.UseDiffuseColor(diffuse);
			if (ImGui::InputFloat3("specular color", &specular[0]))
				globalLight.UseSpecularColor(specular);

			ImGui::PopID();
		}

		for (size_t i = 0; i < scene.PointLights.GetCount(); i++)
		{
			static std::string strLight;
			strLight = "point light #" + std::to_string(i);
			if (ImGui::CollapsingHeader(strLight.c_str(), ImGuiTreeNodeFlags_None))
			{
				ImGui::PushID((int)i);

				auto& pointLight = scene.PointLights[i];
				auto ambientPoint = pointLight.GetAmbientColor();
				auto diffusePoint = pointLight.GetDiffuseColor();
				auto specularPoint = pointLight.GetSpecularColor();
				auto factors = pointLight.GetFactors();

				ImGui::InputFloat3("position", &pointLight.Position[0]);

				if (ImGui::InputFloat3("K factors", &factors[0]))
					pointLight.UseFactors(factors);
				if (ImGui::InputFloat3("ambient color", &ambientPoint[0]))
					pointLight.UseAmbientColor(ambientPoint);
				if (ImGui::InputFloat3("diffuse color", &diffusePoint[0]))
					pointLight.UseDiffuseColor(diffusePoint);
				if (ImGui::InputFloat3("specular color", &specularPoint[0]))
					pointLight.UseSpecularColor(specularPoint);

				ImGui::PopID();
			}
		}

		for (size_t i = 0; i < scene.SpotLights.GetCount(); i++)
		{
			static std::string strLight;
			strLight = "spot light #" + std::to_string(i);
			if (ImGui::CollapsingHeader(strLight.c_str(), ImGuiTreeNodeFlags_None))
			{
				ImGui::PushID(int(i + scene.PointLights.GetCount()));

				auto& spotLight = scene.SpotLights[i];
				auto innerAngle = spotLight.GetInnerAngle();
				auto outerAngle = spotLight.GetOuterAngle();
				auto ambientPoint = spotLight.GetAmbientColor();
				auto diffusePoint = spotLight.GetDiffuseColor();
				auto specularPoint = spotLight.GetSpecularColor();

				ImGui::InputFloat3("position", &spotLight.Position[0]);
				ImGui::InputFloat3("direction", &spotLight.Direction[0]);

				if (ImGui::DragFloat("outer angle", &outerAngle, 1.0f, 0.0f, 90.0f))
					spotLight.UseOuterAngle(outerAngle);
				if (ImGui::DragFloat("inner angle", &innerAngle, 1.0f, 0.0f, 90.0f))
					spotLight.UseInnerAngle(innerAngle);

				if (ImGui::InputFloat3("ambient color", &ambientPoint[0]))
					spotLight.UseAmbientColor(ambientPoint);
				if (ImGui::InputFloat3("diffuse color", &diffusePoint[0]))
					spotLight.UseDiffuseColor(diffusePoint);
				if (ImGui::InputFloat3("specular color", &specularPoint[0]))
					spotLight.UseSpecularColor(specularPoint);

				ImGui::PopID();
			}
		}
    }
}