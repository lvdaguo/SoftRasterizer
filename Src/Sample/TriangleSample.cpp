#include "pch.h"
#include "Head/TriangleSample.h"

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

struct Vertex
{
	vec4 pos;
	vec4 color;
};

static const unsigned int VERTEX_COUNT = 3;
static const unsigned int INDEX_COUNT = 3;

static Vertex vertices[VERTEX_COUNT] =
{
	{ {  0.0f,  1.0f, 0.1f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	{ { -1.0f, -1.0f, 0.1f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ {  1.0f, -1.0f, 0.1f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
};

static unsigned int indices[INDEX_COUNT] = { 0, 1, 2 };

static Ref<VertexBuffer> vb = CreateRef<VertexBuffer>(vertices, VERTEX_COUNT);
static Ref<IndexBuffer> ib = CreateRef<IndexBuffer>(indices, INDEX_COUNT);

static const int VARYING_UV = 0;    // 定义一个 varying 的 key

static VertexShader vs = [&](a2v& v) -> vec4
{
	int index = v.index;
	Vertex* vb = (Vertex*)v.vb;

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

TriangleSample::TriangleSample() { }

void TriangleSample::OnUpdate()
{
	rst.SetClearColor({ 1.0f, 1.0f, 1.0f });
	rst.Clear();

	rst.Bind(vb);
	rst.Bind(ib);
	rst.Bind(vs);
	rst.Bind(fs);
	rst.Draw();
	rst.SwapBuffer();
}
