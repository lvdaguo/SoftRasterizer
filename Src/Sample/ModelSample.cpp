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
#define MODEL_CRITICAL LOG_CRITICAL
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
    }
};

static VertexShader vs = std::bind(VertexShaderSource, std::placeholders::_1);
static FragmentShader fs = std::bind(FragmentShaderSource, std::placeholders::_1);
static ShaderProgram shader = { vs, fs };

static vec3 init_cam_pos = { 0.0f, 0.0f,  8.0f };
static vec3 init_cam_dir = { 0.0f, 0.0f, -1.0f };

#define PAUSE_KEY VK_SPACE
#define TOGGLE_TEXTURE_SAMPLING '3'

static bool is_tex_bilinear = true;
static std::vector<Ref<Texture>> all_textures;

ModelSample::ModelSample() : m_cam(init_cam_pos, init_cam_dir)
{
    auto processInput = [&]()
    {
        if (input.GetKeyDown(PAUSE_KEY))
        {
            app.IsPaused() ? app.Unpause() : app.Pause();
        }
        if (input.GetKeyDown(TOGGLE_TEXTURE_SAMPLING))
        {
            for (auto texture : all_textures) { ToggleSampleMode(*texture); }
        }
    };
    app.InputEvent += { processInput };
}

void ModelSample::OnUpdate()
{
    static Model nanosuit = { "Assets/nanosuit/nanosuit.obj" };
    static Model slimeRabbit = { "Assets/slimes/SlimeRabbit/SlimeRabbit.fbx" };
    static Model slimeKing = { "Assets/slimes/SlimeKing/SlimeKing.fbx" };
    static bool first = true;

    if (first)
    {
        for (auto mesh : nanosuit) { all_textures.push_back(mesh->GetDiffuseTextures().front()); }
        for (auto mesh : slimeRabbit) { all_textures.push_back(mesh->GetDiffuseTextures().front()); }
        for (auto mesh : slimeKing) { all_textures.push_back(mesh->GetDiffuseTextures().front()); }
        first = false;
    }

    rst.SetClearColor({ 0.0f, 0.0f, 0.0f });
    rst.Clear();

    auto drawModel = [](const Model& model)
    {
        for (auto mesh : model)
        {
            rst.Bind(mesh->GetVertexArray());
            rst.Bind(shader);
            rst.Bind(mesh->GetDiffuseTextures().front(), diffuse_slot);
            rst.Draw();
        }
    };

    rst.SetBlend(false);
    u_model_view_projection = m_cam.GetViewProjection() * MatrixLib::Scale(vec3{0.2f}) * mat4(1.0);
    drawModel(nanosuit);

    u_model_view_projection = m_cam.GetViewProjection() * MatrixLib::Translate({ 1.0f, 0.0f, 0.0f }) * MatrixLib::Scale(vec3{0.01f}) * mat4(1.0);
    drawModel(slimeRabbit);

    u_model_view_projection = m_cam.GetViewProjection() * MatrixLib::Translate({ 2.0f, 0.0f, 0.0f }) * MatrixLib::Scale(vec3{0.01f}) * mat4(1.0);
    drawModel(slimeKing);
    
    rst.SwapBuffer();

    MODEL_TRACE("texture sample mode {}", is_tex_bilinear ? "bilinear" : "nearest");
    MODEL_INFO("pos:{} front:{}", m_cam.GetPosition(), m_cam.GetFront());
    if (m_cam.IsPerspective())
    {
        MODEL_TRACE("fov:{} near:{} far:{}", glm::degrees(m_cam.GetFov()), m_cam.GetNear(), m_cam.GetFar());
    }
    else
    {
        MODEL_TRACE("x:{} y:{} z:{}", m_cam.GetXSize(), m_cam.GetYSize(), m_cam.GetZSize());
    }
    MODEL_CRITICAL("{0:.2f}ms, FPS {1:d}", app.GetDeltaTime() * 1000.0f, static_cast<int>(1.0f / app.GetDeltaTime()));
}