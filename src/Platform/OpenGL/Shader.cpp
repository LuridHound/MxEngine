// Copyright(c) 2019 - 2020, #Momo
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

#include "Shader.h"
#include "Utilities/Logging/Logger.h"
#include "Core/Macro/Macro.h"
#include "Platform/OpenGL/GLUtilities.h"
#include "Utilities/FileSystem/File.h"
#include "Core/Config/GlobalConfig.h"
#include "Utilities/Parsing/ShaderPreprocessor.h"

namespace MxEngine
{
	MxString EmptyPath;
	MxVector<MxString> EmptyVector;

	enum class ShaderType
	{
		VERTEX_SHADER   = GL_VERTEX_SHADER,
		GEOMETRY_SHADER = GL_GEOMETRY_SHADER,
		FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
	};

	Shader::Shader()
	{
		this->id = 0;
	}

	void Shader::Bind() const
	{
		GLCALL(glUseProgram(this->id));
		Shader::CurrentlyAttachedShader = this->id;
	}

	void Shader::Unbind() const
	{
		GLCALL(glUseProgram(0));
		Shader::CurrentlyAttachedShader = 0;
	}

    void Shader::InvalidateUniformCache()
    {
		this->uniformCache.clear();
    }

	Shader::BindableId Shader::GetNativeHandle() const
    {
		return this->id;
    }

	Shader::Shader(Shader&& shader) noexcept
	{
		#if defined(MXENGINE_DEBUG)
		this->vertexShaderPath = shader.vertexShaderPath;
		this->geometryShaderPath = shader.geometryShaderPath;
		this->fragmentShaderPath = shader.fragmentShaderPath;
		#endif
		this->id = shader.id;
		this->uniformCache = std::move(shader.uniformCache);
		shader.id = 0;
	}

	Shader& Shader::operator=(Shader&& shader) noexcept
	{
		this->FreeShader();

		#if defined(MXENGINE_DEBUG)
		this->vertexShaderPath = shader.vertexShaderPath;
		this->geometryShaderPath = shader.geometryShaderPath;
		this->fragmentShaderPath = shader.fragmentShaderPath;
		#endif
		this->id = shader.id;
		this->uniformCache = std::move(shader.uniformCache);
		shader.id = 0;
		return *this;
	}

	Shader::~Shader()
	{
		this->FreeShader();
	}

	template<>
	Shader::ShaderId Shader::CompileShader(unsigned int type, const MxString& source, const std::filesystem::path& path)
	{
		GLCALL(GLuint shaderId = glCreateShader((GLenum)type));

		ShaderPreprocessor preprocessor(source);

		auto sourceModified = preprocessor
			.LoadIncludes(FilePath(path.c_str()).parent_path())
			.EmitPrefixLine(Shader::GetShaderVersionString())
			.GetResult()
			;

#if defined(MXENGINE_DEBUG)
		auto& includes = preprocessor.GetIncludeFiles();
		this->includedFilePaths.insert(this->includedFilePaths.end(), includes.begin(), includes.end());
#endif

		auto cStringSource = sourceModified.c_str();
		GLCALL(glShaderSource(shaderId, 1, &cStringSource, nullptr));
		GLCALL(glCompileShader(shaderId));

		GLint result;
		GLCALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result));
		if (result == GL_FALSE)
		{
			GLint length;
			GLCALL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length));
			MxString msg;
			msg.resize(length);
			GLCALL(glGetShaderInfoLog(shaderId, length, &length, &msg[0]));
			msg.pop_back(); // extra \n character
			MxString typeName;
			switch ((ShaderType)type)
			{
			case ShaderType::VERTEX_SHADER:
				typeName = "vertex";
				break;
			case ShaderType::GEOMETRY_SHADER:
				typeName = "geometry";
				break;
			case ShaderType::FRAGMENT_SHADER:
				typeName = "fragment";
				break;
			}
			MXLOG_ERROR("OpenGL::Shader", "failed to compile " + typeName + " shader: " + ToMxString(path));
			MXLOG_ERROR("OpenGL::ErrorHandler", msg);
		}

		return shaderId;
	}

	template<>
	void Shader::Load(const std::filesystem::path& vertex, const std::filesystem::path& fragment)
	{
		this->InvalidateUniformCache();
		this->FreeShader();
		#if defined(MXENGINE_DEBUG)
		this->vertexShaderPath = ToMxString(vertex);
		this->fragmentShaderPath = ToMxString(fragment);
		#endif
		MxString vs = File::ReadAllText(vertex);
		MxString fs = File::ReadAllText(fragment);

		if (vs.empty())
			MXLOG_WARNING("OpenGL::Shader", "vertex shader is empty: " + ToMxString(vertex));
		if (fs.empty())
			MXLOG_WARNING("OpenGL::Shader", "fragment shader is empty: " + ToMxString(fragment));

		MXLOG_DEBUG("OpenGL::Shader", "compiling vertex shader: " + this->vertexShaderPath);
		unsigned int vertexShader = CompileShader((GLenum)ShaderType::VERTEX_SHADER, vs, vertex);
		MXLOG_DEBUG("OpenGL::Shader", "compiling fragment shader: " + this->fragmentShaderPath);
		unsigned int fragmentShader = CompileShader((GLenum)ShaderType::FRAGMENT_SHADER, fs, fragment);

		id = CreateProgram(vertexShader, fragmentShader);
		MXLOG_DEBUG("OpenGL::Shader", "shader program created with id = " + ToMxString(id));
	}

	template<>
	void Shader::Load(const std::filesystem::path& vertex, const std::filesystem::path& geometry, const std::filesystem::path& fragment)
	{
		this->InvalidateUniformCache();
		this->FreeShader();
		#if defined(MXENGINE_DEBUG)
		this->vertexShaderPath   = ToMxString(std::filesystem::proximate(vertex));
		this->geometryShaderPath = ToMxString(std::filesystem::proximate(geometry));
		this->fragmentShaderPath = ToMxString(std::filesystem::proximate(fragment));
		#endif
		MxString vs = File::ReadAllText(vertex);
		MxString gs = File::ReadAllText(geometry);
		MxString fs = File::ReadAllText(fragment);

		if (vs.empty())
			MXLOG_WARNING("OpenGL::Shader", "vertex shader is empty: " + ToMxString(vertex));
		if (gs.empty())
			MXLOG_WARNING("OpenGL::Shader", "geometry shader is empty: " + ToMxString(geometry));
		if (fs.empty())
			MXLOG_WARNING("OpenGL::Shader", "fragment shader is empty: " + ToMxString(fragment));

		MXLOG_DEBUG("OpenGL::Shader", "compiling vertex shader: " + this->vertexShaderPath);
		unsigned int vertexShader = CompileShader((GLenum)ShaderType::VERTEX_SHADER, vs, vertex);
		MXLOG_DEBUG("OpenGL::Shader", "compiling geometry shader: " + this->geometryShaderPath);
		unsigned int geometryShader = CompileShader((GLenum)ShaderType::GEOMETRY_SHADER, gs, geometry);
		MXLOG_DEBUG("OpenGL::Shader", "compiling fragment shader: " + this->fragmentShaderPath);
		unsigned int fragmentShader = CompileShader((GLenum)ShaderType::FRAGMENT_SHADER, fs, fragment);

		id = CreateProgram(vertexShader, geometryShader, fragmentShader);
		MXLOG_DEBUG("OpenGL::Shader", "shader program created with id = " + ToMxString(id));
	}

	void Shader::IgnoreNonExistingUniform(const MxString& name) const
	{
		this->IgnoreNonExistingUniform(name.c_str());
	}

	void Shader::IgnoreNonExistingUniform(const char* name) const
	{
		if (uniformCache.find_as(name) == uniformCache.end())
		{
			GLCALL(int location = glGetUniformLocation(this->id, name));
			uniformCache[name] = location;
		}
	}

    void Shader::LoadFromString(const MxString& vertex, const MxString& fragment)
    {
		this->InvalidateUniformCache();

		MXLOG_DEBUG("OpenGL::Shader", "compiling vertex shader: vertex.glsl");
		unsigned int vertexShader = CompileShader((GLenum)ShaderType::VERTEX_SHADER, vertex, FilePath("vertex.glsl"));
		MXLOG_DEBUG("OpenGL::Shader", "compiling fragment shader: fragment.glsl");
		unsigned int fragmentShader = CompileShader((GLenum)ShaderType::FRAGMENT_SHADER, fragment, FilePath("fragment.glsl"));

		id = CreateProgram(vertexShader, fragmentShader);
		MXLOG_DEBUG("OpenGL::Shader", "shader program created with id = " + ToMxString(id));
    }

	void Shader::LoadFromString(const MxString& vertex, const MxString& geometry, const MxString& fragment)
	{
		this->InvalidateUniformCache();

		MXLOG_DEBUG("OpenGL::Shader", "compiling vertex shader: vertex.glsl");
		unsigned int vertexShader = CompileShader((GLenum)ShaderType::VERTEX_SHADER, vertex, FilePath("vertex.glsl"));
		MXLOG_DEBUG("OpenGL::Shader", "compiling geometry shader: geometry.glsl");
		unsigned int geometryShader = CompileShader((GLenum)ShaderType::GEOMETRY_SHADER, geometry, FilePath("geometry.glsl"));
		MXLOG_DEBUG("OpenGL::Shader", "compiling fragment shader: fragment.glsl");
		unsigned int fragmentShader = CompileShader((GLenum)ShaderType::FRAGMENT_SHADER, fragment, FilePath("fragment.glsl"));

		id = CreateProgram(vertexShader, geometryShader, fragmentShader);
		MXLOG_DEBUG("OpenGL::Shader", "shader program created with id = " + ToMxString(id));
	}

	void Shader::SetUniformFloat(const MxString& name, float f) const
	{
		// shader was not bound before setting uniforms
		MX_ASSERT(Shader::CurrentlyAttachedShader == this->id);

		int location = GetUniformLocation(name);
		if (location == -1) return;
		GLCALL(glUniform1f(location, f));
	}

    void Shader::SetUniformVec2(const MxString& name, const Vector2& vec) const
    {
		// shader was not bound before setting uniforms
		MX_ASSERT(Shader::CurrentlyAttachedShader == this->id);

		int location = GetUniformLocation(name);
		if (location == -1) return;
		GLCALL(glUniform2f(location, vec.x, vec.y));
    }

	void Shader::SetUniformVec3(const MxString& name, const Vector3& vec) const
	{
		// shader was not bound before setting uniforms
		MX_ASSERT(Shader::CurrentlyAttachedShader == this->id);

		int location = GetUniformLocation(name);
		if (location == -1) return;
		GLCALL(glUniform3f(location, vec.x, vec.y, vec.z));
	}

	void Shader::SetUniformVec4(const MxString& name, const Vector4& vec) const
	{
		// shader was not bound before setting uniforms
		MX_ASSERT(Shader::CurrentlyAttachedShader == this->id);

		int location = GetUniformLocation(name);
		if (location == -1) return;
		GLCALL(glUniform4f(location, vec.x, vec.y, vec.z, vec.w));
	}

	void Shader::SetUniformMat4(const MxString& name, const Matrix4x4& matrix) const
	{
		int location = GetUniformLocation(name);
		if (location == -1) return;
		Bind();
		GLCALL(glUniformMatrix4fv(location, 1, false, &matrix[0][0]));
	}

	void Shader::SetUniformMat3(const MxString& name, const Matrix3x3& matrix) const
	{
		// shader was not bound before setting uniforms
		MX_ASSERT(Shader::CurrentlyAttachedShader == this->id);
		int location = GetUniformLocation(name);
		if (location == -1) return;
		GLCALL(glUniformMatrix3fv(location, 1, false, &matrix[0][0]));
	}

	void Shader::SetUniformInt(const MxString& name, int i) const
	{
		// shader was not bound before setting uniforms
		MX_ASSERT(Shader::CurrentlyAttachedShader == this->id);
		int location = GetUniformLocation(name);
		if (location == -1) return;
		GLCALL(glUniform1i(location, i));
	}

	void Shader::SetUniformBool(const MxString& name, bool b) const
	{
		this->SetUniformInt(name, (int)b);
	}

	const MxString& Shader::GetVertexShaderDebugFilePath() const
	{
		#if defined(MXENGINE_DEBUG)
		return this->vertexShaderPath;
		#else
		return EmptyPath;
		#endif
	}

	const MxString& Shader::GetGeometryShaderDebugFilePath() const
	{
		#if defined(MXENGINE_DEBUG)
		return this->geometryShaderPath;
		#else
		return EmptyPath;
		#endif
	}

	const MxString& Shader::GetFragmentShaderDebugFilePath() const
	{
		#if defined(MXENGINE_DEBUG)
		return this->fragmentShaderPath;
		#else
		return EmptyPath;
		#endif
	}

	const MxVector<MxString>& Shader::GetIncludedFilePaths() const
	{
		#if defined(MXENGINE_DEBUG)
		return this->includedFilePaths;
		#else
		return EmptyVector;
		#endif
	}

	Shader::BindableId Shader::CreateProgram(Shader::ShaderId vertexShader, Shader::ShaderId fragmentShader) const
	{
		GLCALL(unsigned int program = glCreateProgram());

		GLCALL(glAttachShader(program, vertexShader));
		GLCALL(glAttachShader(program, fragmentShader));
		GLCALL(glLinkProgram(program));
		GLCALL(glValidateProgram(program));

		GLCALL(glDeleteShader(vertexShader));
		GLCALL(glDeleteShader(fragmentShader));

		return program;
	}

	Shader::BindableId Shader::CreateProgram(ShaderId vertexShader, ShaderId geometryShader, ShaderId fragmentShader) const
	{
		GLCALL(unsigned int program = glCreateProgram());

		GLCALL(glAttachShader(program, vertexShader));
		GLCALL(glAttachShader(program, geometryShader));
		GLCALL(glAttachShader(program, fragmentShader));
		GLCALL(glLinkProgram(program));
		GLCALL(glValidateProgram(program));

		GLCALL(glDeleteShader(vertexShader));
		GLCALL(glDeleteShader(geometryShader));
		GLCALL(glDeleteShader(fragmentShader));

		return program;
	}

	int Shader::GetUniformLocation(const MxString& uniformName) const
	{
		if (uniformCache.find(uniformName) != uniformCache.end())
			return uniformCache[uniformName];

		GLCALL(int location = glGetUniformLocation(this->id, uniformName.c_str()));
		if (location == -1)
		{
			#if defined(MXENGINE_DEBUG)
			MXLOG_WARNING("OpenGL::Shader", '[' + this->fragmentShaderPath + "]: " + "uniform was not found: " + uniformName);
			#else
			MXLOG_WARNING("OpenGL::Shader", "uniform was not found: " + uniformName);
			#endif
		}
		uniformCache[uniformName] = location;
		return location;
	}

	template<>
	Shader::Shader(const std::filesystem::path& vertexShaderPath, const std::filesystem::path& fragmentShaderPath)
	{
		this->Load(vertexShaderPath, fragmentShaderPath);
	}

	template<>
	Shader::Shader(const std::filesystem::path& vertexShaderPath, const std::filesystem::path& geometryShaderPath, const std::filesystem::path& fragmentShaderPath)
	{
		this->Load(vertexShaderPath, geometryShaderPath, fragmentShaderPath);
	}

	void Shader::FreeShader()
	{
		if (id != 0)
		{
			GLCALL(glDeleteProgram(id));
		}
	}

    MxString Shader::GetShaderVersionString()
    {
		return "#version " + ToMxString(GlobalConfig::GetGraphicAPIMajorVersion() * 100 + GlobalConfig::GetGraphicAPIMinorVersion() * 10);
    }
}