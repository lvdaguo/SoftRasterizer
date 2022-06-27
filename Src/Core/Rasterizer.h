#pragma once

#include "Core/Graphics/Shader.h"

#include "Core/Buffer/IndexBuffer.h"
#include "Core/Buffer/VertexBuffer.h"
#include "Core/Buffer/VertexArray.h"

#include "Core/Buffer/ColorBuffer.h"
#include "Core/Buffer/DepthBuffer.h"

struct Rect { vec2i min, max; };

// 顶点顺序为逆时针
struct Triangle
{
	union
	{ 
		struct { Vertex* a, *b, *c; }; 
		Vertex* v[3];
	};
};

const unsigned int MAX_TEXTURE_SLOT = 32;

enum class MSAALevel
{
	None = 1, // 1代表子采样点数目为1，即关闭MSAA
	X2 = 2,
	X4 = 4,
	X8 = 8,
	X16 = 16
};

class Rasterizer : public Singleton<Rasterizer>
{
public:
	virtual ~Rasterizer() override;
	void Init(unsigned int threadCount, MSAALevel msaaLevel = MSAALevel::None);

public:
	void Draw();

	void Clear();
	void SwapBuffer();

private:
	void NaiveDraw();
	void NaiveDrawMSAA();
	void MultiThreadDraw();

	void* CreateBitMap(HWND hWnd);

private:
	void OnRender();

public:
	void Bind(Ref<VertexBuffer> vertexBuffer) { m_vertexBuffer = vertexBuffer; }
	void Bind(Ref<IndexBuffer> indexBuffer) { m_indexBuffer = indexBuffer; }
	void Bind(Ref<VertexArray> vertexArray) { m_vertexBuffer = vertexArray->GetVertexBuffer(); m_indexBuffer = vertexArray->GetIndexBuffer(); }
	void Bind(VertexShader vs) { m_vertexShader = vs; }
	void Bind(FragmentShader fs) { m_fragmentShader = fs; }
	void Bind(ShaderProgram shader) { m_vertexShader = shader.GetVertexShader(); m_fragmentShader = shader.GetFragmentShader(); }
	void Bind(Ref<Texture> texture, unsigned int slot = 0) { m_textureSlots[slot] = texture; }

public:
	void SetDepthTest(bool enable) { m_depthTest = enable; }
	void SetBlend(bool enable) { m_blend = enable; }
	void SetCullFace(bool cullFront) { m_isCullFront = cullFront; }

	void SetClearColor(const vec3& color) { m_clearColor = color; }

	void SetMSAALevel(MSAALevel level) 
	{ 
		m_msaaLevel = level; 
		unsigned int subsampleCount = static_cast<unsigned int>(level);
		m_depthBuffer->SetSubsampleCount(subsampleCount);
	}

public:
	MSAALevel GetMSAALevel() const { return m_msaaLevel; }

private:
	VertexShader m_vertexShader;
	FragmentShader m_fragmentShader;

private:
	bool m_isCullFront;

	bool m_depthTest;
	bool m_blend;

	MSAALevel m_msaaLevel;

private:
	Ref<Texture> m_textureSlots[MAX_TEXTURE_SLOT];

private:
	Ref<VertexBuffer> m_vertexBuffer;
	Ref<IndexBuffer> m_indexBuffer;

	Ref<ColorBuffer> m_colorBuffer;
	Ref<DepthBuffer> m_depthBuffer;
	vec3 m_clearColor;

private:
	unsigned int m_workThreadCount, m_childWorkThreadCount;
	unsigned int m_multiThreadThreshold;
	ThreadPool m_threadPool;

private:
	HDC m_hDC, m_hMem;
	unsigned int m_width, m_height;
	Action m_onRender;
};
