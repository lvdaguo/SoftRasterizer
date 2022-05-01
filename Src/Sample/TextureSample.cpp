#include "pch.h"
#include "TextureSample.h"

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

static const unsigned int vertexCount = 4;
static const unsigned int indexCount = 6;

// ��������
// �Ȼ���͸���ģ�λ�ýϺ�
// ��͸���ģ�λ�����ǰ

static vertex vertices1[vertexCount] =
{
	{ {  0.5f,  0.5f, 0.5f, 1.f }, { 1.0f, 1.0f } },
	{ { -0.5f,  0.5f, 0.5f, 1.f }, { 0.0f, 1.0f } },
	{ { -0.5f, -0.5f, 0.5f, 1.f }, { 0.0f, 0.0f } },
	{ {  0.5f, -0.5f, 0.5f, 1.f }, { 1.0f, 0.0f } }
};

static vertex vertices2[vertexCount] =
{
	{ {  0.5f + 0.2f,  0.5f + 0.2f, 0.2f, 1.f }, { 1.0f, 1.0f } },
	{ { -0.5f + 0.2f,  0.5f + 0.2f, 0.2f, 1.f }, { 0.0f, 1.0f } },
	{ { -0.5f + 0.2f, -0.5f + 0.2f, 0.2f, 1.f }, { 0.0f, 0.0f } },
	{ {  0.5f + 0.2f, -0.5f + 0.2f, 0.2f, 1.f }, { 1.0f, 0.0f } }
};

static unsigned int indices[indexCount] = { 0, 1, 2, 0, 2, 3 };

static Ref<VertexBuffer> vb1 = CreateRef<VertexBuffer>(vertices1, vertexCount);
static Ref<VertexBuffer> vb2 = CreateRef<VertexBuffer>(vertices2, vertexCount);

static Ref<IndexBuffer> ib = CreateRef<IndexBuffer>(indices, indexCount);

static Ref<Texture> texture = CreateRef<Texture>("Asset/emoji.jpg");
static Ref<Texture> glass = CreateRef<Texture>("Asset/blending_transparent_window.png");

static const int VARYING_UV = 0;    // ����һ�� varying �� key

static vec4 VertexShaderSource(a2v& v)
{
	int index = v.index;
	vertex* vb = (vertex*)v.vb;
	vec4& pos = vb[index].pos;
	vec2& uv = vb[index].uv;
	vec2& out_uv = v.f2[VARYING_UV];

	out_uv = uv;
	return pos;
}

static unsigned int textureSlot = 0;

static vec4 FragmentShaderSource(v2f& i)
{
	vec2& uv = i.f2[VARYING_UV];
	Texture& texture = *i.textures[textureSlot];

	vec4 out_color = sample2D(texture, uv);
	return out_color;
}

static VertexShader vs = std::bind(VertexShaderSource, std::placeholders::_1);
static FragmentShader fs = std::bind(FragmentShaderSource, std::placeholders::_1);
static ShaderProgram shader = { vs, fs };

TextureSample::TextureSample()
{

}

void TextureSample::OnUpdate()
{
	rst.SetClearColor({ 0.0f, 0.0f, 0.0f });
	rst.Clear();
	
	rst.Bind(shader);

	rst.Bind(vb1);
	rst.Bind(ib);
	rst.Bind(texture);
	rst.Draw();

	rst.Bind(vb2);
	rst.Bind(ib);
	rst.Bind(glass);
	rst.Draw();

	rst.SwapBuffer();
}