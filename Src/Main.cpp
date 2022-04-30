#include "pch.h"
#include "params.h"

#include "Core/Graphics/Shader.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Rasterizer.h"

#include "Sample/TriangleSample.h"
#include "Sample/DepthBlendSample.h"

static Window& window = Window::Instance();
static Application& app = Application::Instance();
static Rasterizer& rst = Rasterizer::Instance();

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	app.Init(APP_NAME);
	window.Init(hInstance, WIDTH, HEIGHT, app);
	rst.Init();

	Sample* sp = new TriangleSample();
	std::function<void()> update = [&]() { sp->OnUpdate(); };
	
	Action updateCallback = Action(update);
	app.AppUpdateEvent += updateCallback;
	app.Run();
	app.AppUpdateEvent -= updateCallback;

	return 0;
}