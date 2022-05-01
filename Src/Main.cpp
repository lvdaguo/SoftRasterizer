#include "pch.h"
#include "params.h"

#include "Core/Graphics/Shader.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Rasterizer.h"

#include "Sample/TriangleSample.h"
#include "Sample/DepthBlendSample.h"
#include "Sample/TextureSample.h"
#include "Sample/BoxSample.h"

static Window& window = Window::Instance();
static Application& app = Application::Instance();
static Rasterizer& rst = Rasterizer::Instance();

void TestSample(Sample* sp);
void TestAllSamples();

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	app.Init(APP_NAME);
	window.Init(hInstance, WIDTH, HEIGHT, app);
	rst.Init();

	//TestSample(new TextureSample());
	TestAllSamples();

	return 0;
}

void TestSample(Sample* sp)
{
	std::function<void()> update = [&]() { sp->OnUpdate(); };

	Action updateCallback = Action(update);
	app.AppUpdateEvent += updateCallback;
	app.Run();
	app.AppUpdateEvent -= updateCallback;
}

void TestAllSamples()
{
	const unsigned int maxSampleFrame = 4;

	std::vector<Sample*> sp;
	sp.push_back(new TriangleSample());
	sp.push_back(new DepthBlendSample());
	sp.push_back(new TextureSample());

	Sample* cur = sp.front();
	int i = 0, count = maxSampleFrame;
	std::function<void()> update = [&]() 
	{ 
		if (count) count--;
		else
		{
			i = (i + 1) % sp.size();
			cur = sp[i];
			count = maxSampleFrame;
		}
		cur->OnUpdate();
	};

	Action updateCallback = Action(update);
	app.AppUpdateEvent += updateCallback;
	app.Run();
	app.AppUpdateEvent -= updateCallback;
}