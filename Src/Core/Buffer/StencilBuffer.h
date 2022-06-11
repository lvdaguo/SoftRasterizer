#pragma once

#include "pch.h"

class StencilBuffer
{
public:
	StencilBuffer(unsigned int width, unsigned int height)
		: m_width(width), m_height(height)
	{
		//m_buffer = new float[width * height];
		m_buffer = std::vector<bool>(width * height, false);
		Clear();
	}

	~StencilBuffer()
	{
		//delete[] m_buffer;
	}

public:
	bool GetStencil(unsigned int x, unsigned int y)
	{
		// yֵ�����ڵڼ��У�xֵ�����ڵڼ���
		return m_buffer[y * m_width + x];
	}

	void SetStencil(unsigned int x, unsigned int y, bool stencil)
	{
		m_buffer[y * m_width + x] = stencil;
	}

	void Clear()
	{
		// һ��һ��
		for (unsigned int i = 0; i < m_width * m_height; ++i)
		{
			m_buffer[i] = std::numeric_limits<float>::max();
		}
	}

private:
	std::vector<bool> m_buffer;
	unsigned int m_width, m_height;
};