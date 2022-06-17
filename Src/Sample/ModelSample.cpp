#include "pch.h"

#include "Head/ModelSample.h"

#include "Core/MatrixTool.h"
#include "Core/Buffer/VertexArray.h"

#include "Core/Graphics/Texture.h"
#include "Core/Graphics/Shader.h"

#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Rasterizer.h"
#include "Core/Input.h"

#include "Extension/Model.h"

#define window Window::Instance()
#define app Application::Instance()
#define rst Rasterizer::Instance()
#define input Input::Instance()

#define MODEL_LOG

#ifdef MODEL_LOG
#define MODEL_TRACE LOG_TRACE
#define MODEL_INFO LOG_INFO
#define MODEl_CRITICAL LOG_CRITICAL
#else
#define MODEL_TRACE 
#define MODEL_INFO 
#define MODEL_CRITICAL 
#endif

// uniform
static mat4 u_model_view_projection;

static unsigned int VARYING_UV = 0;

static vec4 VertexShaderSource(a2v& v)
{
    int index = v.index;
    VertexData* vb = (VertexData*)v.vb;

    // in
    vec3& position = vb[index].position;
    vec3& normal = vb[index].normal;
    vec2& uv = vb[index].texCoords;

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

static unsigned int diffuse_slot = 1;

static vec4 FragmentShaderSource(v2f& i)
{
    // in
    vec2& uv = i.f2[VARYING_UV];

    // uniform
    Texture& diffuse = *i.textures[diffuse_slot];

    // main()
    {
        vec4 out_color = Sample2D(diffuse, uv);
        return out_color;
        //return { 1.0f, 0.0f, 0.0f, 1.0f };
    }
};

static VertexShader vs = std::bind(VertexShaderSource, std::placeholders::_1);
static FragmentShader fs = std::bind(FragmentShaderSource, std::placeholders::_1);
static ShaderProgram shader = { vs, fs };

static vec3 init_cam_pos = { 0.0f, 0.0f,  8.0f };
static vec3 init_cam_dir = { 0.0f, 0.0f, -1.0f };

ModelSample::ModelSample() : m_cam(init_cam_pos, init_cam_dir)
{
    auto processInput = [&]()
    {
        if (input.GetKeyDown(VK_SPACE))
        {
            app.IsPaused() ? app.Unpause() : app.Pause();
        }
    };
    app.InputEvent += { processInput };
}

void ModelSample::OnUpdate()
{
    static Model nanosuit = { "Asset/nanosuit/nanosuit.obj" };
    static Model slimeRabbit = { "Asset/slimes/SlimeRabbit/SlimeRabbit.fbx" };
    static Model slimeKing = { "Asset/slimes/SlimeKing/SlimeKing.fbx" };

    rst.Clear();
    rst.SetClearColor({ 0.0f, 0.0f, 0.0f });

    rst.SetBlend(false);
    rst.Bind(shader);
    u_model_view_projection = m_cam.GetViewProjection() * MatrixLib::Scale(vec3{0.2f}) * mat4(1.0);
    for (auto mesh : nanosuit)
    {
        rst.Bind(mesh->GetVertexArray());
        rst.Bind(mesh->GetDiffuseTextures().front(), diffuse_slot);
        rst.Draw();
    }

    u_model_view_projection = m_cam.GetViewProjection() * 
        MatrixLib::Translate({ 1.0f, 0.0f, 0.0f }) *  
        MatrixLib::Scale(vec3{0.01f}) * mat4(1.0);
    for (auto mesh : slimeRabbit)
    {
        rst.Bind(mesh->GetVertexArray());
        rst.Bind(mesh->GetDiffuseTextures().front(), diffuse_slot);
        rst.Draw();
    }

    u_model_view_projection = m_cam.GetViewProjection() *
        MatrixLib::Translate({ 2.0f, 0.0f, 0.0f }) *
        MatrixLib::Scale(vec3{0.01f}) * mat4(1.0);
    for (auto mesh : slimeKing)
    {
        rst.Bind(mesh->GetVertexArray());
        rst.Bind(mesh->GetDiffuseTextures().front(), diffuse_slot);
        rst.Draw();
    }

    rst.SwapBuffer();

    MODEL_INFO("pos:{} front:{}", m_cam.GetPosition(), m_cam.GetFront());
    if (m_cam.IsPerspective())
    {
        MODEL_TRACE("fov:{} near:{} far:{}", glm::degrees(m_cam.GetFov()), m_cam.GetNear(), m_cam.GetFar());
    }
    else
    {
        MODEL_TRACE("x:{} y:{} z:{}", m_cam.GetXSize(), m_cam.GetYSize(), m_cam.GetZSize());
    }
    MODEl_CRITICAL("{0:.2f}ms, FPS {1:d}", app.GetDeltaTime() * 1000.0f, static_cast<int>(1.0f / app.GetDeltaTime()));
}