#include "pch.h"
#include "Head/CameraSample.h"

#include "Core/MatrixTool.h"
#include "Core/Buffer/VertexArray.h"

#include "Core/Graphics/Texture.h"
#include "Core/Graphics/Shader.h"

#include "Core/Camera.h"

#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Rasterizer.h"

#define window Window::Instance()
#define app Application::Instance()
#define rst Rasterizer::Instance()

struct vertex
{
    vec3 pos;
    vec2 uv;
};

static const unsigned int VERTEX_COUNT = 36;
static const unsigned int INDEX_COUNT = 36;

static vertex vertices[VERTEX_COUNT] =
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

static unsigned int indices[INDEX_COUNT] =
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

static Ref<VertexBuffer> vb = CreateRef<VertexBuffer>(vertices, VERTEX_COUNT);
static Ref<IndexBuffer> ib = CreateRef<IndexBuffer>(indices, INDEX_COUNT);
static Ref<VertexArray> va = CreateRef<VertexArray>(vb, ib);

// uniform
static mat4 u_model_view_projection;
static Ref<Texture> u_texture = CreateRef<Texture>("Assets/jile.jpg");

// VARYING_KEY
static const int VARYING_UV = 0;

static vec4 VertexShaderSource(a2v& v)
{
    int index = v.index;
    vertex* vb = (vertex*)v.vb;

    // in
    vec3& position = vb[index].pos;
    vec2& uv = vb[index].uv;

    // out
    vec2& out_uv = v.f2[VARYING_UV];

    // uniform
    mat4& mvp = u_model_view_projection;

    // main()
    {
        vec4 pos = vec4(position, 1.0f);
        pos = mvp * pos;
        out_uv = uv;
        return pos;
    }
};

static unsigned int texture_slot = 0; // 默认槽位为0

static vec4 FragmentShaderSource(v2f& i)
{
    // in
    vec2& uv = i.f2[VARYING_UV];

    // uniform
    Texture& texture = *i.textures[texture_slot];

    // main()
    {
        vec4 out_color = Sample2D(texture, uv);
        return out_color;
    }
};

static VertexShader vs = std::bind(VertexShaderSource, std::placeholders::_1);
static FragmentShader fs = std::bind(FragmentShaderSource, std::placeholders::_1);
static ShaderProgram shader = { vs, fs };

static vec3 init_cam_pos = { 0.0f, 2.0f,  6.0f };
static vec3 init_cam_dir = { 0.0f, 0.0f,  -1.0f };

static Camera cam = { init_cam_pos, init_cam_dir };

static float rotation = 0.0f;

CameraSample::CameraSample() { }

void CameraSample::OnUpdate()
{
    LOG_CRITICAL("{0:.2f}ms, FPS {1:d}", app.GetDeltaTime() * 1000.0f, static_cast<int>(1.0f / app.GetDeltaTime()));
    rst.Clear();
    rst.SetClearColor({ 1.0f, 1.0f, 1.0f });

    rst.Bind(va);
    rst.Bind(shader);
    rst.Bind(u_texture);

    mat4 model;

    // A方盒绕着原点公转，同时自转
    mat4 m1 = MatrixLib::Rotate(glm::radians(rotation), { 1.0f, 1.0f, 1.0f });
    mat4 m2 = MatrixLib::Translate({ 1.5f, 0.0f, 0.0f });
    mat4 m3 = MatrixLib::Rotate(glm::radians(rotation), { 0.0f, 1.0f, 0.0f });
    model = m3 * m2 * m1;

    vec4 box_center = { 0.0f, 0.0f, 0.0f, 1.0f };
    box_center = model * box_center;
    box_center /= box_center.w;
    vec3 look_pos = vec3(box_center);
    vec3 cam_dir = look_pos - cam.GetPosition();
    cam.SetFront(cam_dir);

    // 摄像机始终看向A方盒子的位置
    u_model_view_projection = cam.GetViewProjection() * model;
    rst.Draw();

    // B方盒在原点自转
    mat4 m4 = MatrixLib::Scale({ 1.0f, 2.0f, 1.0f });
    mat4 m5 = MatrixLib::Rotate(glm::radians(rotation), { 0.0f, 1.0f, 0.0f });
    model = m5 * m4;
    u_model_view_projection = cam.GetViewProjection() * model;
    rst.Draw();

    rotation += 1.0f;
    rst.SwapBuffer();
}
