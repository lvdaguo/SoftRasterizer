#pragma once

#include "Core/Graphics/Color.h"
#include "Core/Graphics/Shader.h"

#include "Core/Buffer/IndexBuffer.h"
#include "Core/Buffer/VertexBuffer.h"
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

class Rasterizer : public Singleton<Rasterizer>
{
public:
	Rasterizer() { }
	virtual ~Rasterizer() override;
	void Init(VertexShader vs = NULL, FragmentShader fs = NULL);

public:
	void Draw();

	void Clear();
	void SwapBuffer();

public:
	void Bind(Ref<VertexBuffer> vertexBuffer) { m_vertexBuffer = vertexBuffer; }
	void Bind(Ref<IndexBuffer> indexBuffer) { m_indexBuffer = indexBuffer; }
	void Bind(VertexShader vs) { m_vertexShader = vs; }
	void Bind(FragmentShader fs) { m_fragmentShader = fs; }

public:
	void SetClearColor(const vec3& color) { m_clearColor = color; }
	void SetWireFrameColor(const vec3& color) { m_wireFrameColor = color; }
	void SetDrawMode(bool isWireFrame) { m_drawWireFrame = isWireFrame; }

private:
	void* CreateBitMap(HWND hWnd);

	void DrawLine(vec2 p1, vec2 p2);

	void OnUpdate();

private:
	VertexShader m_vertexShader;
	FragmentShader m_fragmentShader;

private:
	Action m_onUpdate;

	bool m_drawWireFrame;
	vec3 m_wireFrameColor;

private:
	Ref<VertexBuffer> m_vertexBuffer;
	Ref<IndexBuffer> m_indexBuffer;

	Ref<ColorBuffer> m_colorBuffer;
	Ref<DepthBuffer> m_depthBuffer;
	vec3 m_clearColor;

private:
	HDC m_hDC, m_hMem;
	unsigned int m_width, m_height;
};