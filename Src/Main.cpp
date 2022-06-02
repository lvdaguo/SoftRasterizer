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

#define logger Logger::Instance()
#define app Application::Instance()
#define window Window::Instance()
#define rst Rasterizer::Instance()
#define input Input::Instance()

void TestSample(Sample* sp);
void TestAllSamples();

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	logger.Init();

	app.Init(L"SoftRasterizer");
	window.Init(hInstance, 800, 600, app);
	rst.Init(12);
	input.Init();

	//TestSample(new TriangleSample());
	//TestSample(new DepthBlendSample());
	//TestSample(new TextureSample()); 
	TestSample(new BoxTransformSample());
	//TestSample(new CameraSample());
	//TestSample(new FreeDemoSample());

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
	const unsigned int maxSampleFrame = 10;

	std::vector<Sample*> sp;
	sp.push_back(new BoxTransformSample());
	sp.push_back(new TriangleSample());
	sp.push_back(new DepthBlendSample());
	sp.push_back(new TextureSample());
	sp.push_back(new CameraSample());

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