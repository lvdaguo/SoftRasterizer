#include "pch.h"
#include "Head/BoxTransformSample.h"

#include "Core/MatrixTool.h"
#include "Core/Buffer/VertexArray.h"

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
	vec3 pos;
	vec2 uv;
};

static const unsigned int vertexCount = 36;
static const unsigned int indexCount = 36;

static vertex vertices[vertexCount] =
{
    { { -0.5f, -0.5f, -0.5f },  { 0.0f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f },  { 1.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f },  { 1.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f },  { 1.0f, 1.0f } },
    { { -0.5f,  0.5f, -0.5f },  { 0.0f, 1.0f } },
    { { -0.5f, -0.5f, -0.5f },  { 0.0f, 0.0f } },

    { { -0.5f, -0.5f,  0.5f },  { 0.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f },  { 1.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f },  { 1.0f, 1.0f } },
    { {  0.5f,  0.5f,  0.5f },  { 1.0f, 1.0f } },
    { { -0.5f,  0.5f,  0.5f },  { 0.0f, 1.0f } },
    { { -0.5f, -0.5f,  0.5f },  { 0.0f, 0.0f } },
    
    { { -0.5f,  0.5f,  0.5f },  { 1.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f },  { 1.0f, 1.0f } },
    { { -0.5f, -0.5f, -0.5f },  { 0.0f, 1.0f } },
    { { -0.5f, -0.5f, -0.5f },  { 0.0f, 1.0f } },
    { { -0.5f, -0.5f,  0.5f },  { 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f },  { 1.0f, 0.0f } },

    { {  0.5f,  0.5f,  0.5f },  { 1.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f },  { 1.0f, 1.0f } },
    { {  0.5f, -0.5f, -0.5f },  { 0.0f, 1.0f } },
    { {  0.5f, -0.5f, -0.5f },  { 0.0f, 1.0f } },
    { {  0.5f, -0.5f,  0.5f },  { 0.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f },  { 1.0f, 0.0f } },

    { { -0.5f, -0.5f, -0.5f },  { 0.0f, 1.0f } },
    { {  0.5f, -0.5f, -0.5f },  { 1.0f, 1.0f } },
    { {  0.5f, -0.5f,  0.5f },  { 1.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f },  { 1.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f },  { 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f },  { 0.0f, 1.0f } },

    { { -0.5f,  0.5f, -0.5f },  { 0.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f },  { 1.0f, 1.0f } },
    { {  0.5f,  0.5f,  0.5f },  { 1.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f },  { 1.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f },  { 0.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f },  { 0.0f, 1.0f } }
};

static unsigned int indices[indexCount] = 
{ 
	0, 2, 1,
	3, 5, 4, 
	6, 7, 8, 
	9, 10, 11,
    12, 13, 14,
    15, 16, 17,
    18, 20, 19,
    21, 23, 22,
    24, 25, 26,
    27, 28, 29,
    30, 32, 31,
    33, 35, 34
};

static Ref<VertexBuffer> vb = CreateRef<VertexBuffer>(vertices, vertexCount);
static Ref<IndexBuffer> ib = CreateRef<IndexBuffer>(indices, indexCount);
static Ref<VertexArray> va = CreateRef<VertexArray>(vb, ib);

// uniform
static mat4 u_model_view_projection;
static Ref<Texture> u_texture = CreateRef<Texture>("Asset/jile.jpg");

// VARYING_KEY
static const int VARYING_UV = 0;
static const int VARYING_Z = 1;

static vec4 VertexShaderSource(a2v& v)
{
	int index = v.index;
	vertex* vb = (vertex*)v.vb;

    // in
    vec3& position = vb[index].pos;
	vec2& uv = vb[index].uv;
	
    // out
    vec2& out_uv = v.f2[VARYING_UV];
    float& out_z = v.f1[VARYING_Z];

    // uniform
    mat4& mvp = u_model_view_projection;

    // main()
    {
        vec4 pos = vec4(position, 1.0f);
        pos = mvp * pos;
        out_uv = uv;
        out_z = pos.z / pos.w;
        return pos;
    }
};

static unsigned int textureSlot = 0; // 默认槽位为0

static vec4 FragmentShaderSource(v2f& i)
{
    // in
	vec2& uv = i.f2[VARYING_UV];
    float& z = i.f1[VARYING_Z];

    // uniform
    Texture& texture = *i.textures[textureSlot];

    // main()
    {
        vec4 out_color = sample2D(texture, uv);
        //vec4 out_color = { z, z, z, 1.0f };
        return out_color;
    }
};

static VertexShader vs = std::bind(VertexShaderSource, std::placeholders::_1);
static FragmentShader fs = std::bind(FragmentShaderSource, std::placeholders::_1);
static ShaderProgram shader = { vs, fs };

static vec3 eye_pos = { 0.0f, 2.0f, 8.0f };
static vec3 at = { 0.0f, 0.0f, 0.0f };
static vec3 up = { 0.0f, 1.0f, 0.0f };

static float fov = glm::radians(45.0f);
static float aspect; // 不能在此计算，此时窗口单例还未初始化
static float n = 0.1f, f = 50.0f;

static mat4 model;
static mat4 view;
static mat4 projection;

static void InitMatrix()
{
    model = mat4(1.0f);
    aspect = (float)window.GetWidth() / (float)window.GetHeight();
    view = MatrixTool::look_at(eye_pos, at, up);
    projection = MatrixTool::perspective(fov, aspect, n, f);
    //projection = MatrixTool::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 20.0f);
    u_model_view_projection = projection * view * model;
}

BoxTransformSample::BoxTransformSample() 
{ 
    InitMatrix();
}

static float rotation = 0.0f;

void BoxTransformSample::OnUpdate()
{
	rst.Clear();
    rst.SetClearColor({ 1.0f, 1.0f, 1.0f });
    
    rst.Bind(va);
    rst.Bind(shader);
    rst.Bind(u_texture);

    // A方盒绕着原点公转，同时自转
    mat4 m1 = MatrixTool::rotate(glm::radians(rotation), { 1.0f, 1.0f, 1.0f });
    mat4 m2 = MatrixTool::translate({ 3.0f, 0.0f, 0.0f });
    mat4 m3 = MatrixTool::rotate(glm::radians(rotation), { 0.0f, 1.0f, 0.0f });
    model = m3 * m2 * m1;

    vec4 box_center = { 0.0f, 0.0f, 0.0f, 1.0f };
    box_center = model * box_center;
    box_center /= box_center.w;
    at = vec3(box_center);

    // 摄像机始终看向A方盒子的位置
    view = MatrixTool::look_at(eye_pos, at, up);
    u_model_view_projection = projection * view * model;

	rst.Draw();

    // B方盒在原点自转
    mat4 m4 = MatrixTool::scale({ 1.0f, 2.0f, 1.0f });
    mat4 m5 = MatrixTool::rotate(glm::radians(rotation), { 0.0f, 1.0f, 0.0f });
    model = m5 * m4;
    u_model_view_projection = projection * view * model;
    rst.Draw();

    rotation += 1.0f;

    rst.SwapBuffer();
}
