#pragma once

#include "pch.h"
#include "Core/Graphics/Shader.h"

struct Vertex
{
	a2v appdata;
	vec4 pos;			// 坐标
	vec3 spos;			// 浮点数屏幕坐标
	float rhw;			// w 的倒数
	bool discard;
};

class VertexBuffer
{
public:
	VertexBuffer(void* data, unsigned int count) : m_data(data), m_count(count)
	{
		m_buffer = new Vertex[count];
		for (unsigned int i = 0; i < count; ++i)
		{
			m_buffer[i].appdata.vb = data;
		}
	}

	~VertexBuffer()
	{
		delete[] m_buffer;
	}

public:
	Vertex& operator[](unsigned int index)
	{
		return m_buffer[index];
	}

	unsigned int GetCount() const { return m_count; }

public:
	Vertex* begin() { return m_buffer; }
	Vertex* end() { return m_buffer + m_count; }
	const Vertex* begin() const { return m_buffer; }
	const Vertex* end() const { return m_buffer + m_count; }

private:
	void* m_data;
	Vertex* m_buffer;
	unsigned int m_count;
};