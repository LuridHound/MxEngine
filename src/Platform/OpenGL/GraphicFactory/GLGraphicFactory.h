// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in sourceand binary forms, with or without
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

#include "Core/Interfaces/GraphicAPI/GraphicFactory.h"
#include "Platform/OpenGL/GraphicModule/GLGraphicModule.h"
#include "Platform/OpenGL/Window/GLWindow.h"
#include "Platform/OpenGL/IndexBuffer/GLIndexBuffer.h"
#include "Platform/OpenGL/Shader/GLShader.h"
#include "Platform/OpenGL/Texture/GLTexture.h"
#include "Platform/OpenGL/VertexBuffer/GLVertexBuffer.h"
#include "Platform/OpenGL/VertexArray/GLVertexArray.h"
#include "Platform/OpenGL/VertexBufferLayout/GLVertexBufferLayout.h"
#include "Platform/OpenGL/Renderer/GLRenderer.h"

namespace MxEngine
{
    class GLGraphicFactory final : public GraphicFactory
    {
        inline virtual Renderer& GetRenderer() override
        {
            static GLRenderer renderer;
            return renderer;
        }

        inline virtual GraphicModule& GetGraphicModule() override
        {
            static GLGraphicModule module;
            return module;
        }

        inline virtual UniqueRef<Window> CreateWindow() override
        {
            return UniqueRef<Window>(Alloc<GLWindow>());
        }

        inline virtual UniqueRef<IndexBuffer> CreateIndexBuffer() override
        {
            return UniqueRef<IndexBuffer>(Alloc<GLIndexBuffer>());
        }

        inline virtual UniqueRef<Shader> CreateShader() override
        {
            return UniqueRef<Shader>(Alloc<GLShader>());
        }

        inline virtual UniqueRef<Texture> CreateTexture() override
        {
            return UniqueRef<Texture>(Alloc<GLTexture>());
        }

        inline virtual UniqueRef<VertexArray> CreateVertexArray() override
        {
            return UniqueRef<VertexArray>(Alloc<GLVertexArray>());
        }

        inline virtual UniqueRef<VertexBuffer> CreateVertexBuffer() override
        {
            return UniqueRef<VertexBuffer>(Alloc<GLVertexBuffer>());
        }

        inline virtual UniqueRef<VertexBufferLayout> CreateVertexBufferLayout() override
        {
            return UniqueRef<VertexBufferLayout>(Alloc<GLVertexBufferLayout>());
        }

        inline virtual UniqueRef<Window> CreateWindow(int width, int height, const std::string& title)
        {
            auto window = this->CreateWindow();
            window->UseSize(width, height);
            window->UseTitle(title);
            return window;
        }

        inline virtual UniqueRef<IndexBuffer> CreateIndexBuffer(const IndexBuffer::IndexBufferType& data) override
        {
            auto ibo = this->CreateIndexBuffer();
            ibo->Load(data);
            return std::move(ibo);
        }

        inline virtual UniqueRef<Shader> CreateShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) override
        {
            auto shader = this->CreateShader();
            shader->Load(vertexShaderPath, fragmentShaderPath);
            return std::move(shader);
        }

        inline virtual UniqueRef<Texture> CreateTexture(const std::string& filepath, bool genMipmaps = true, bool flipImage = true) override
        {
            auto texture = this->CreateTexture();
            texture->Load(filepath, genMipmaps, flipImage);
            return std::move(texture);
        }

        inline virtual UniqueRef<VertexBuffer> CreateVertexBuffer(const VertexBuffer::BufferData& data, UsageType type) override
        {
            auto vbo = this->CreateVertexBuffer();
            vbo->Load(data, type);
            return std::move(vbo);
        }
    };
}