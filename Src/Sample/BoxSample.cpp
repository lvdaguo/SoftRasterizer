#include "pch.h"
#include "BoxSample.h"

#include "Core/Graphics/Texture.h"
#include "Core/Graphics/Shader.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Rasterizer.h"

static Window& window = Window::Instance();
static Application& app = Application::Instance();
static Rasterizer& rst = Rasterizer::Instance();

struct vertex
{
	vec4 pos;
	vec2 uv;
};

static const unsigned int vertexCount = 8;
static const unsigned int indexCount = 36;

// 两个方块
// 先画不透明的，位置较后
// 后画透明的，位置相对前

static vertex vertices[vertexCount] =
{
	{ {  1.0f, -1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f } },
	{ { -1.0f, -1.0f,  1.0f,  1.0f }, { 0.0f, 1.0f } },
	{ { -1.0f,  1.0f,  1.0f,  1.0f }, { 1.0f, 1.0f } },
	{ {  1.0f,  1.0f,  1.0f,  1.0f }, { 1.0f, 0.0f } },
	{ {  1.0f, -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f } },
	{ { -1.0f, -1.0f, -1.0f,  1.0f }, { 0.0f, 1.0f } },
	{ { -1.0f,  1.0f, -1.0f,  1.0f }, { 1.0f, 1.0f } },
	{ {  1.0f,  1.0f, -1.0f,  1.0f }, { 1.0f, 0.0f } }
};

static unsigned int indices[indexCount] = 
{ 
	0, 2, 1,
	0, 3, 2, 
	4, 5, 6, 
	4, 6, 7 
};

static Ref<VertexBuffer> vb = CreateRef<VertexBuffer>(vertices, vertexCount);
static Ref<IndexBuffer> ib = CreateRef<IndexBuffer>(indices, indexCount);

static Texture texture = Texture("Asset/emoji.jpg");
static const int VARYING_COLOR = 0;

static VertexShader vs = [&](a2v& v) -> vec4
{
	int index = v.index;
	vertex* vb = (vertex*)v.vb;
	vec4& pos = vb[index].pos;
	vec2& uv = vb[index].uv;
	vec4& out_color = v.f4[VARYING_COLOR];

	out_color = sample2D(texture, uv);
	return pos;
};

static FragmentShader fs = [&](v2f& i) -> vec4
{
	vec4& in_color = i.f4[VARYING_COLOR];
	return in_color;
};

BoxSample::BoxSample()
{
	rst.SetClearColor({ 1.0f, 1.0f, 1.0f });
	rst.Bind(vb);
	rst.Bind(ib);
	rst.Bind(vs);
	rst.Bind(fs);
}

void BoxSample::OnUpdate()
{
	rst.Clear();
	rst.Draw();
	rst.SwapBuffer();
}
