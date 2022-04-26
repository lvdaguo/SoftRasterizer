#pragma once

#include "Core/GlobalHeaders.h"

class ColorBuffer
{
public:
	ColorBuffer(void* bmpBuffer, unsigned int width, unsigned int height) 
		: m_buffer((Color*)bmpBuffer), m_width(width), m_height(height)
	{ }

	~ColorBuffer()
	{
		// delete m_buffer;	
	} // bmpBuffer 会由系统自动释放，不需要自己delete

	void SetColor(unsigned int x, unsigned int y, Color color)
	{
		// y值代表在第几行，x值代表在第几列
		m_buffer[y * m_width + x] = color;
	}

	void FillColor(Color color)
	{
		// 一行一行
		for (unsigned int y = 0; y < m_height; ++y)
		{
			for (unsigned int x = 0; x < m_width; ++x)
			{
				m_buffer[y * m_width + x] = color;
			}
		}
	}

private:
	Color* m_buffer;
	unsigned int m_width;
	unsigned int m_height;
};