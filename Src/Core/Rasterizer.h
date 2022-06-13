#pragma once

#include "Core/Graphics/Shader.h"

#include "Core/Buffer/IndexBuffer.h"
#include "Core/Buffer/VertexBuffer.h"
#include "Core/Buffer/VertexArray.h"

#include "Core/Buffer/ColorBuffer.h"
#include "Core/Buffer/DepthBuffer.h"

struct Rect { vec2i min, max; };

// ¶¥µãË³ÐòÎªÄæÊ±Õë
struct Triangle
{
	union
	{ 
		struct { Vertex* a, *b, *c; }; 
		Vertex* v[3];
	};
};

const unsigned int MAX_TEXTURE_SLOT = 32;

class Rasterizer : public Singleton<Rasterizer>
{
public:
	virtual ~Rasterizer() override;
	void Init(unsigned int threadCount);

public:
	void Draw();

	void Clear();
	void SwapBuffer();

private:
	void NaiveDraw();
	void MultiThreadDraw();

	void* CreateBitMap(HWND hWnd);
	void DrawLine(const vec2& p1, const vec2& p2);

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

	void SetClearColor(const vec3& color) { m_clearColor = color; }
	void SetWireFrameColor(const vec3& color) { m_wireFrameColor = color; }
	void SetDrawMode(bool isWireFrame) { m_drawWireFrame = isWireFrame; }

private:
	VertexShader m_vertexShader;
	FragmentShader m_fragmentShader;

private:
	bool m_drawWireFrame;
	vec3 m_wireFrameColor;

	bool m_depthTest;
	bool m_blend;

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
