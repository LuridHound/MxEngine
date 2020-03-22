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

#include "Application.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/Math/Math.h"
#include "Utilities/Profiler/Profiler.h"
#include "Core/Interfaces/GraphicAPI/GraphicFactory.h"
#include "Core/Event/Event.h"
#include "Core/Camera/PerspectiveCamera.h"
#include "Core/Camera/OrthographicCamera.h"

// conditional includes
#include "Core/Macro/Macro.h"

#include "Platform/OpenGL/GraphicFactory/GLGraphicFactory.h"
#include "Library/Scripting/Python/PythonEngine.h"
#include "Library/Primitives/Colors.h"
#include "Utilities/Format/Format.h"

namespace MxEngine
{
	#define DECL_EVENT_TYPE(event) dispatcher.RegisterEventType<event>()
	void InitEventDispatcher(AppEventDispatcher& dispatcher)
	{
		DECL_EVENT_TYPE(AppDestroyEvent);
		DECL_EVENT_TYPE(FpsUpdateEvent);
		DECL_EVENT_TYPE(KeyEvent);
		DECL_EVENT_TYPE(MouseMoveEvent);
		DECL_EVENT_TYPE(RenderEvent);
		DECL_EVENT_TYPE(UpdateEvent);
	}

	Application::Application()
		: manager(this), window(Graphics::Instance()->CreateWindow(1280, 720, "MxEngine Application")),
		timeDelta(0), counterFPS(0), renderer(Graphics::Instance()->GetRenderer())
	{
		this->GetWindow().UseEventDispatcher(&this->dispatcher);
		this->CreateScene("Global", MakeUnique<Scene>("Global", "Resources/"));
		this->CreateScene("Default", MakeUnique<Scene>("Default", "Resources/"));
		this->LoadScene("Default");

		InitEventDispatcher(this->GetEventDispatcher());

		#define FORWARD_EVENT_SCENE(event)\
		this->dispatcher.AddEventListener<event>(#event, [this](event& e)\
		{\
			this->GetCurrentScene().GetEventDispatcher().Invoke(e);\
		})
		FORWARD_EVENT_SCENE(AppDestroyEvent);
		FORWARD_EVENT_SCENE(FpsUpdateEvent);
		FORWARD_EVENT_SCENE(KeyEvent);
		FORWARD_EVENT_SCENE(MouseMoveEvent);
		FORWARD_EVENT_SCENE(RenderEvent);
		FORWARD_EVENT_SCENE(UpdateEvent);
	}

	void Application::ToggleMeshDrawing(bool state)
	{
		this->debugMeshDraw = state;
	}

	void Application::OnCreate()
	{
		//is overriden in derived class
	}

	void Application::OnUpdate()
	{
		// is overriden in derived class
	}

	void Application::OnDestroy()
	{
		// is overriden in derived class
	}

	Window& Application::GetWindow()
	{
		return *this->window;
	}

	Scene& Application::GetCurrentScene()
	{
		return *this->currentScene;
	}

	Scene& Application::GetGlobalScene()
	{
		assert(this->scenes.Exists("Global"));
		return *this->scenes.Get("Global");
	}

    void Application::LoadScene(const std::string& name)
    {
		if (name == this->GetGlobalScene().GetName())
		{
			Logger::Instance().Error("MxEngine::Application", "global scene cannot be loaded: " + name);
			return;
		}
		if (!this->scenes.Exists(name))
		{
			Logger::Instance().Error("MxEngine::Application", "cannot load scene as it does not exist: " + name);
			return;
		}
		// unload previous scene if it exists
		if (this->currentScene != nullptr) 
			this->currentScene->OnUnload();

		this->currentScene = this->scenes.Get(name);
		this->currentScene->OnLoad();
    }

	void Application::DestroyScene(const std::string& name)
	{
		if (name == this->GetGlobalScene().GetName())
		{
			Logger::Instance().Error("MxEngine::Application", "trying to destroy global scene: " + name);
			return;
		}
		if (!this->scenes.Exists(name))
		{
			Logger::Instance().Warning("MxEngine::Application", "trying to destroy not existing scene: " + name);
			return;
		}
		if (this->scenes.Get(name) == this->currentScene)
		{
			Logger::Instance().Error("MxEngine::Application", "cannot destroy scene which is used: " + name);
			return;
		}
		this->scenes.Delete(name);
	}

	Scene& Application::CreateScene(const std::string& name, UniqueRef<Scene> scene)
	{
		if (scenes.Exists(name))
		{
			Logger::Instance().Error("MxEngine::Application", "scene with such name already exists: " + name);
		}
		else
		{
			scene->GetEventDispatcher() = this->GetEventDispatcher().Clone();
			scenes.Add(name, std::move(scene));
			scenes.Get(name)->OnCreate();
		}
		return *scenes.Get(name);
	}

	Scene& Application::GetScene(const std::string& name)
	{
		assert(this->scenes.Exists(name));
		return *this->scenes.Get(name);
	}

	bool Application::SceneExists(const std::string& name)
	{
		return this->scenes.Exists(name);
	}

    Counter::CounterType Application::GenerateResourceId()
    {
		return this->resourceIdCounter++;
    }

    float Application::GetTimeDelta() const
	{
		return this->timeDelta;
	}

    int Application::GetCurrentFPS() const
    {
		return this->counterFPS;
    }

	AppEventDispatcher& Application::GetEventDispatcher()
	{
		return this->dispatcher;
	}

	RenderController& Application::GetRenderer()
	{
		return this->renderer;
	}

	LoggerImpl& Application::GetLogger()
	{
		return Logger::Instance();
	}

	void Application::ExecuteScript(const Script& script)
	{
		MAKE_SCOPE_PROFILER("Application::ExecuteScript");
		auto& engine = this->GetConsole().GetEngine();
		engine.Execute(script.GetContent());
		if (engine.HasErrors())
		{
			Logger::Instance().Error("Application::ExecuteScript", engine.GetErrorMessage());
		}
	}

	void Application::ToggleDeveloperConsole(bool isVisible)
	{
		this->GetConsole().Toggle(isVisible);
		this->GetWindow().UseEventDispatcher(isVisible ? nullptr : &this->dispatcher);
	}

	void Application::DrawObjects(bool meshes) const
	{
		MAKE_SCOPE_PROFILER("Application::DrawObjects");
		const auto& viewport = this->currentScene->Viewport;

		LightSystem lights;
		lights.Global = this->currentScene->GlobalLight;
		lights.Point = this->currentScene->PointLights.GetView();
		lights.Spot = this->currentScene->SpotLights.GetView();

		for (const auto& object : this->currentScene->GetObjectList())
		{
			this->renderer.DrawObject(*object.second, viewport, lights);
		}
		if (meshes)
		{
			for (const auto& object : this->currentScene->GetObjectList())
			{
				this->renderer.DrawObjectMesh(*object.second, viewport);
			}
		}
	}

	void Application::InvokeUpdate()
	{
		this->GetWindow().OnUpdate();
		MAKE_SCOPE_PROFILER("MxEngine::OnUpdate");
		UpdateEvent updateEvent(this->timeDelta);
		this->GetEventDispatcher().Invoke(updateEvent);
		for (auto& [_, object] : this->currentScene->GetObjectList())
		{
			object->OnUpdate();
		}
		this->currentScene->OnUpdate();
		this->OnUpdate();
		this->currentScene->OnRender();
		this->currentScene->PrepareRender();
	}

	bool Application::VerifyApplicationState()
	{
		if (!this->GetWindow().IsCreated())
		{
			Logger::Instance().Error("MxEngine::Application", "window was not created, aborting...");
			return false;
		}
		if (this->isRunning)
		{
			Logger::Instance().Error("MxEngine::Application", "Application::Run() is called when application is already running");
			return false;
		}
		if (!this->GetWindow().IsOpen())
		{
			Logger::Instance().Error("MxEngine::Application", "window was created but is closed. Note that application can be runned only once");
			return false;
		}
		return true; // verified!
	}

	void Application::VerifyRendererState()
	{
		auto& Renderer = this->GetRenderer();
		if (Renderer.DefaultTexture == nullptr)
		{
			Renderer.DefaultTexture = Colors::MakeTexture(Colors::WHITE);
		}
		if (Renderer.ObjectShader == nullptr)
		{
			Renderer.ObjectShader = Application::Get()->GetGlobalScene().GetResourceManager<Shader>().Add(
				"MxObjectShader", Graphics::Instance()->CreateShader());
			Renderer.ObjectShader->LoadFromSource(
				#include "Core/Shaders/object_vertex.glsl"
				,
				#include "Core/Shaders/object_fragment.glsl"
			);
		}
		if (Renderer.MeshShader == nullptr)
		{
			Renderer.MeshShader = Application::Get()->GetGlobalScene().GetResourceManager<Shader>().Add(
				"MxMeshShader", Graphics::Instance()->CreateShader());
			Renderer.MeshShader->LoadFromSource(
				#include "Core/Shaders/mesh_vertex.glsl"
				,		 
				#include "Core/Shaders/mesh_fragment.glsl"
			);
		}
	}

	void Application::CloseApplication()
	{
		this->shouldClose = true;
	}

	void Application::CreateContext()
	{
		if (this->GetWindow().IsCreated())
		{
			Logger::Instance().Warning("MxEngine::Application", "CreateContext() called when window was already created");
			return;
		}
		MAKE_SCOPE_PROFILER("Application::CreateContext");
		this->GetWindow()
			.UseProfile(3, 3, Profile::CORE)
			.UseCursorMode(CursorMode::DISABLED)
			.UseSampling(4)
			.UseDoubleBuffering(false)
			.UseTitle("MxEngine Project")
			.UsePosition(600, 300)
			.Create();

		auto& renderingEngine = this->renderer.GetRenderEngine();
		renderingEngine
			.UseDepthBuffer()
			.UseCulling()
			.UseSampling()
			.UseClearColor(0.0f, 0.0f, 0.0f)
			.UseTextureMagFilter(MagFilter::NEAREST)
			.UseTextureMinFilter(MinFilter::LINEAR_MIPMAP_LINEAR)
			.UseTextureWrap(WrapType::REPEAT, WrapType::REPEAT)
			.UseBlending(BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA)
			.UseAnisotropicFiltering(renderingEngine.GetLargestAnisotropicFactor());

		this->CreateConsoleBindings(this->GetConsole());
	}

	DeveloperConsole& Application::GetConsole()
	{
		return this->console;
	}

	void Application::Run()
	{
		if (!VerifyApplicationState()) return;
		this->isRunning = true;

		if (this->GetConsole().IsToggled())
		{
			this->GetConsole().Log("Welcome to MxEngine developer console!");
			#if defined(MXENGINE_USE_PYTHON)
			this->GetConsole().Log("This console is powered by Python: https://www.python.org");
			#endif
		}

		{
			MAKE_SCOPE_PROFILER("Application::OnCreate");
			MAKE_SCOPE_TIMER("MxEngine::Application", "Application::OnCreate()");
			this->OnCreate();
		}
		float secondEnd = Time::Current(), frameEnd = Time::Current();
		int fpsCounter = 0;
		VerifyRendererState();
		{
			MAKE_SCOPE_PROFILER("Application::Run");
			MAKE_SCOPE_TIMER("MxEngine::Application", "Application::Run()");
			Logger::Instance().Debug("MxEngine::Application", "starting main loop...");
			while (this->GetWindow().IsOpen())
			{
				fpsCounter++;
				float now = Time::Current();
				if (now - secondEnd >= 1.0f)
				{
					this->counterFPS = fpsCounter;
					fpsCounter = 0;
					secondEnd = now;
					this->GetEventDispatcher().AddEvent(MakeUnique<FpsUpdateEvent>(this->counterFPS));
				}
				timeDelta = now - frameEnd;
				frameEnd = now;

				// event phase
				{
					MAKE_SCOPE_PROFILER("Application::ProcessEvents");
					this->GetEventDispatcher().InvokeAll();
					this->currentScene->GetEventDispatcher().InvokeAll();
					if (this->shouldClose) break;
				}

				this->InvokeUpdate();
				this->GetRenderer().Clear();
				this->DrawObjects(this->debugMeshDraw);

				RenderEvent renderEvent;
				this->GetEventDispatcher().Invoke(renderEvent);
				this->renderer.Render();
				this->GetWindow().PullEvents(); 
				if (this->shouldClose) break;
			}

			// application exit
			{
				MAKE_SCOPE_PROFILER("Application::CloseApplication");
				MAKE_SCOPE_TIMER("MxEngine::Application", "Application::CloseApplication()");
				this->currentScene->OnUnload();
				AppDestroyEvent appDestroyEvent;
				this->GetEventDispatcher().Invoke(appDestroyEvent);
				this->OnDestroy();
				this->GetWindow().Close();
				this->isRunning = false;
			}
		}
	}

	bool Application::IsRunning() const
	{
		return this->isRunning;
	}

	Application::~Application()
	{
		{
			MAKE_SCOPE_PROFILER("Application::DestroyObjects");
			MAKE_SCOPE_TIMER("MxEngine::Application", "Application::DestroyObjects");

			for (auto& scene : this->scenes.GetStorage())
			{
				scene.second->OnDestroy();
			}
			this->scenes.Clear();
		}
		Logger::Instance().Debug("MxEngine::Application", "application destroyed");
	}

	Application* Application::Get()
	{
		return Application::Current;
	}

	void Application::Set(Application* application)
	{
		Application::Current = application;
	}

	Application::ModuleManager::ModuleManager(Application* app)
	{
		Profiler::Instance().StartSession("profile_log.json");
		
		assert(Application::Get() == nullptr);
		Application::Set(app);

		#if defined(MXENGINE_USE_OPENGL)
		Graphics::Instance() = Alloc<GLGraphicFactory>();
		#else
		Graphics::Instance() = nullptr;
		Logger::Instance().Error("MxEngine::Application", "No Rendering Engine was provided");
		return;
		#endif
		Graphics::Instance()->GetGraphicModule().Init();
	}

	Application::ModuleManager::~ModuleManager()
	{
		Graphics::Instance()->GetGraphicModule().Destroy();
		Profiler::Instance().EndSession();
	}

	#if defined(MXENGINE_USE_PYTHON)
	void Application::CreateConsoleBindings(DeveloperConsole& console)
	{
		console.SetSize({ this->GetWindow().GetWidth() / 2.5f, this->GetWindow().GetHeight() / 2.0f });
		this->GetEventDispatcher().AddEventListener<RenderEvent>("DeveloperConsole",
			[this](RenderEvent&) { this->GetConsole().OnRender(); });
	}
	#else
	void Application::CreateConsoleBindings(DeveloperConsole& console)
	{
		console.SetSize({ this->GetWindow().GetWidth() / 2.5f, this->GetWindow().GetHeight() / 2.0f });
		this->GetEventDispatcher().AddEventListener<RenderEvent>("DeveloperConsole",
			[this](RenderEvent&) { this->GetConsole().OnRender(); });
	}
	#endif
}