#include "pch.h"
#include "params.h"

#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Rasterizer.h"

Window& window = Window::Instance();
Application& app = Application::Instance();
Rasterizer& rst = Rasterizer::Instance();

// 顶点数据，由 VS 读取，如有多个三角形，可每次更新 vs_input 再绘制
struct { vec4 pos; vec4 color; } vs_input[3] = 
{
	{ {  0.0,  0.7, 0.90, 1}, {1, 0, 0, 1} },
	{ { -0.6, -0.2, 0.01, 1}, {0, 1, 0, 1} },
	{ { +0.6, -0.2, 0.01, 1}, {0, 0, 1, 1} },
};

const int VARYING_COLOR = 0;    // 定义一个 varying 的 key

VertexShader vs = [&](int i, v2f& out) -> vec4
{
	vec4& pos = vs_input[i].pos;
	vec4& color = vs_input[i].color;
	vec4& out_color = out.vec4[VARYING_COLOR];

	out_color = color;
	return pos;
};

FragmentShader fs = [&](v2f& in) -> vec4
{
	vec4& in_color = in.vec4[VARYING_COLOR];

	return in_color;
};

std::function<void()> update = [&]()
{
	// 进行下一帧绘制前需要清屏
	rst.Clear();
	rst.DrawTriangle();
	rst.SwapBuffer();
};

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	app.Init(APP_NAME);
	window.Init(hInstance, WIDTH, HEIGHT, app);
	rst.Init(vs, fs);

	Action updateCallback = Action(update);
	app.AppUpdateEvent += updateCallback;

	app.Run();

	app.AppUpdateEvent -= updateCallback;

	return 0;
}