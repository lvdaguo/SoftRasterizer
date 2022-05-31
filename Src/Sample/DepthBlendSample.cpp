#include "pch.h"
#include "Head/DepthBlendSample.h"

#include "Core/Graphics/Shader.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Rasterizer.h"

//static Window& window = Window::Instance();
//static Application& app = Application::Instance();
//static Rasterizer& rst = Rasterizer::Instance();

#define window Window::Instance()
#define app Application::Instance()
#define rst Rasterizer::Instance()

struct vertex 
{ 
	vec4 pos; 
	vec4 color; 
};

static const unsigned int VERTEX_COUNT = 4;
static const unsigned int INDEX_COUNT = 6;

// 两个方块
// 先画不透明的，位置较后
// 后画透明的，位置相对前

static vertex vertices1[VERTEX_COUNT] =
{
	{ {  0.5f,  0.5f, 0.5f, 1.f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f,  0.5f, 0.5f, 1.f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, -0.5f, 0.5f, 1.f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
	{ {  0.5f, -0.5f, 0.5f, 1.f }, { 0.0f, 1.0f, 1.0f, 1.0f } }
};

static vertex vertices2[VERTEX_COUNT] =
{
	{ {  0.5f + 0.2f,  0.5f + 0.2f, 0.4f, 1.f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ { -0.5f + 0.2f,  0.5f + 0.2f, 0.4f, 1.f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
	{ { -0.5f + 0.2f, -0.5f + 0.2f, 0.4f, 1.f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
	{ {  0.5f + 0.2f, -0.5f + 0.2f, 0.4f, 1.f }, { 0.0f, 1.0f, 0.0f, 1.0f } }
};

static unsigned int indices[INDEX_COUNT] = { 0, 1, 2, 0, 2, 3 };

static Ref<VertexBuffer> vb1 = CreateRef<VertexBuffer>(vertices1, VERTEX_COUNT);
static Ref<VertexBuffer> vb2 = CreateRef<VertexBuffer>(vertices2, VERTEX_COUNT);
static Ref<IndexBuffer> ib = CreateRef<IndexBuffer>(indices, INDEX_COUNT);

static const int VARYING_UV = 0;    // 定义一个 varying 的 key

static VertexShader vs = [&](a2v& v) -> vec4
{
	int index = v.index;
	vertex* vb = (vertex*)v.vb;

	// in
	vec4& pos = vb[index].pos;
	vec4& color = vb[index].color;
	
	// out
	vec4& out_color = v.f4[VARYING_UV];

	// main()
	{
		out_color = color;
		return pos;
	}
};

static FragmentShader fs = [&](v2f& i) -> vec4
{
	// in
	vec4& in_color = i.f4[VARYING_UV];

	// main()
	{
		return in_color;
	}
};

DepthBlendSample::DepthBlendSample() { }

void DepthBlendSample::OnUpdate()
{
	rst.SetClearColor({ 1.0f, 1.0f, 1.0f });
	rst.Clear();

	rst.Bind(ib);
	rst.Bind(vs);
	rst.Bind(fs);
	rst.Bind(vb1);
	rst.Draw();
	rst.Bind(vb2);
	rst.Draw();
	rst.SwapBuffer();
}