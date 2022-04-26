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
	} // bmpBuffer ����ϵͳ�Զ��ͷţ�����Ҫ�Լ�delete

	void SetColor(unsigned int x, unsigned int y, Color color)
	{
		// yֵ�����ڵڼ��У�xֵ�����ڵڼ���
		m_buffer[y * m_width + x] = color;
	}

	void FillColor(Color color)
	{
		// һ��һ��
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