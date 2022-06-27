#include "pch.h"

#include "Core/Graphics/Shader.h"

#include "Core/Logger.h"

#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Rasterizer.h"
#include "Core/Input.h"

#include "Sample/Head/TriangleSample.h"
#include "Sample/Head/DepthBlendSample.h"
#include "Sample/Head/TextureSample.h"
#include "Sample/Head/BoxTransformSample.h"
#include "Sample/Head/CameraSample.h"
#include "Sample/Head/FreeDemoSample.h"
#include "Sample/Head/ModelSample.h"

#define logger Logger::Instance()
#define app Application::Instance()
#define window Window::Instance()
#define rst Rasterizer::Instance()
#define input Input::Instance()

#define SAMPLE_SWITCH_UP VK_OEM_MINUS
#define SAMPLE_SWITCH_DOWN VK_OEM_PLUS

#define SCREEN_WIDTH  960
#define SCREEN_HEIGHT 540
// #define SCREEN_WIDTH  800
// #define SCREEN_HEIGHT 600
#define WORK_THREAD_COUNT 12

void TestSample(Sample* sp);
void TestAllSamples();

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	logger.Init();

	app.Init(L"SoftRasterizer");
	window.Init(hInstance, SCREEN_WIDTH, SCREEN_HEIGHT, app);
	rst.Init(WORK_THREAD_COUNT);
	input.Init();

	//TestSample(new TriangleSample());
	//TestSample(new DepthBlendSample());
	//TestSample(new TextureSample()); 
	//TestSample(new BoxTransformSample());
	//TestSample(new CameraSample());
	//TestSample(new FreeDemoSample());
	TestSample(new ModelSample());

	//TestAllSamples();

	return 0;
}

void TestSample(Sample* sp)
{
	Action updateCallback = { std::bind(&Sample::OnUpdate, sp) };
	app.AppUpdateEvent += updateCallback;
	app.Run();
	app.AppUpdateEvent -= updateCallback;
}

void TestAllSamples()
{
	std::vector<Sample*> sp;
	sp.push_back(new FreeDemoSample());
	sp.push_back(new ModelSample());
	sp.push_back(new TriangleSample());
	sp.push_back(new DepthBlendSample());
	sp.push_back(new TextureSample());
	sp.push_back(new BoxTransformSample());
	sp.push_back(new CameraSample());


	Sample* cur = sp.front();
	int i = 0;

	std::function<void()> onInput = [&]()
	{
		if (input.GetKeyDown(SAMPLE_SWITCH_UP))
		{
			i = (i + sp.size() - 1) % sp.size();
			cur = sp[i];
		}
		if (input.GetKeyDown(SAMPLE_SWITCH_DOWN))
		{
			i = (i + sp.size() + 1) % sp.size();
			cur = sp[i];
		}
	};

	std::function<void()> onUpdate = [&](){ cur->OnUpdate(); };

	Action onInputCallback = { onInput };
	Action onUpdateCallback = { onUpdate };
	
	app.InputEvent += onInputCallback;
	app.AppUpdateEvent += onUpdateCallback;
	
	app.Run();
	
	app.AppUpdateEvent -= onUpdateCallback;
	app.InputEvent -= onInputCallback;
}