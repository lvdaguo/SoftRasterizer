#include "pch.h"
#include "params.h"

#include "Core/Graphics/Shader.h"

#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Rasterizer.h"

Window& window = Window::Instance();
Application& app = Application::Instance();
Rasterizer& rst = Rasterizer::Instance();

// 顶点数据，由 VS 读取，如有多个三角形，可每次更新 vertices 再绘制
struct { vec4 pos; vec4 color; } vertices[8] = 
{
	{ {  0.5f,  0.5f, 0.5f, 1.f}, {0.0f, 1.0f, 1.0f, 1.0f} },
	{ { -0.5f,  0.5f, 0.5f, 1.f}, {0.0f, 1.0f, 1.0f, 1.0f} },
	{ { -0.5f, -0.5f, 0.5f, 1.f}, {0.0f, 1.0f, 1.0f, 1.0f} },
	{ {  0.5f, -0.5f, 0.5f, 1.f}, {0.0f, 1.0f, 1.0f, 1.0f} },

	{ {  0.5f + 0.2f,  0.5f + 0.2f, 0.4f, 1.f}, {0.0f, 1.0f, 0.0f, 1.0f} },
	{ { -0.5f + 0.2f,  0.5f + 0.2f, 0.4f, 1.f}, {0.0f, 1.0f, 0.0f, 0.0f} },
	{ { -0.5f + 0.2f, -0.5f + 0.2f, 0.4f, 1.f}, {0.0f, 1.0f, 0.0f, 0.0f} },
	{ {  0.5f + 0.2f, -0.5f + 0.2f, 0.4f, 1.f}, {0.0f, 1.0f, 0.0f, 1.0f} }
};

unsigned int indices[12] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 };

const int VARYING_COLOR = 0;    // 定义一个 varying 的 key

VertexShader vs = [&](a2v& v) -> vec4
{
	int index = v.index;
	vec4& pos = vertices[index].pos;
	vec4& color = vertices[index].color;
	vec4& out_color = v.f4[VARYING_COLOR];

	out_color = color;
	return pos;
};

FragmentShader fs = [&](v2f& i) -> vec4
{
	vec4& in_color = i.f4[VARYING_COLOR];
	return in_color;
};

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	app.Init(APP_NAME);
	window.Init(hInstance, WIDTH, HEIGHT, app);
	rst.Init(vs, fs);

	Ref<VertexBuffer> vb = CreateRef<VertexBuffer>(8);
	Ref<IndexBuffer> ib = CreateRef<IndexBuffer>(indices, 12);

	rst.Bind(vb);
	rst.Bind(ib);

	std::function<void()> update = [&]()
	{
		// 进行下一帧绘制前需要清屏
		rst.SetClearColor({ 1.0f, 1.0f, 1.0f });
		rst.Clear();
		rst.Draw();
		rst.SwapBuffer();
	};

	Action updateCallback = Action(update);
	app.AppUpdateEvent += updateCallback;
	app.Run();
	app.AppUpdateEvent -= updateCallback;

	return 0;
}