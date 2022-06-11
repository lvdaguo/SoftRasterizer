#pragma once

#include "pch.h"

class DepthBuffer
{
public:
	DepthBuffer(unsigned int width, unsigned int height)
		: m_width(width), m_height(height)
	{ 
		m_buffer = new float[width * height];
		Clear();
	}

	~DepthBuffer()
	{
		delete[] m_buffer;
	}

public:
	float GetDepth(unsigned int x, unsigned int y)
	{
		// yֵ�����ڵڼ��У�xֵ�����ڵڼ���
		return m_buffer[y * m_width + x];
	}

	void SetDepth(unsigned int x, unsigned int y, float depth)
	{
		// yֵ�����ڵڼ��У�xֵ�����ڵڼ���
		m_buffer[y * m_width + x] = depth;
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
	float* m_buffer;
	unsigned int m_width, m_height;
};