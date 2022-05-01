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
	float& data(unsigned int x, unsigned int y)
	{
		// y值代表在第几行，x值代表在第几列
		return m_buffer[y * m_width + x];
	}

	void Clear()
	{
		// 一行一行
		for (unsigned int y = 0; y < m_height; ++y)
		{
			for (unsigned int x = 0; x < m_width; ++x)
			{
				data(x, y) = std::numeric_limits<float>::infinity();
			}
		}
	}

private:
	float* m_buffer;
	unsigned int m_width, m_height;
};