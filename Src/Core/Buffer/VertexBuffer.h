#pragma once

#include "Core/Graphics/Shader.h"

struct Vertex
{
	a2v appdata;
	vec4 pos;			// ����
	vec3 spos;			// ��������Ļ����
	float rhw;			// w �ĵ���
};

class VertexBuffer
{
public:
	VertexBuffer(unsigned int count) : m_count(count)
	{
		m_buffer = new Vertex[count];
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

	Vertex* begin() { return m_buffer; }
	Vertex* end() { return m_buffer + m_count; }
	const Vertex* begin() const { return m_buffer; }
	const Vertex* end() const { return m_buffer + m_count; }

private:
	Vertex* m_buffer;
	unsigned int m_count;
};