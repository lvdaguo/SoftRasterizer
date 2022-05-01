#pragma once

#include "pch.h"
#include "Core/Graphics/Color.h"

class ColorBuffer
{
public:
	ColorBuffer(void* bmpBuffer, unsigned int width, unsigned int height) 
		: m_buffer((Color*)bmpBuffer), m_width(width), m_height(height)
	{ }

	~ColorBuffer()
	{
		// delete[] m_buffer;	
	} // bmpBuffer 会由系统自动释放，不需要自己delete

public:
	void SetColor(unsigned int x, unsigned int y, const vec3& color)
	{
		// y值代表在第几行，x值代表在第几列
		m_buffer[(m_height - 1 - y) * m_width + x] = { byte(color.r * 255.f), byte(color.g * 255.f), byte(color.b * 255.f), 255 };
	}

	vec3 GetColor(unsigned int x, unsigned int y)
	{
		Color& color = m_buffer[(m_height - 1 - y) * m_width + x];
		return { (float)color.r / 255.f, (float)color.g / 255.f, (float)color.b / 255.f };
	}

	void FillColor(const vec3& color)
	{
		// 一行一行
		for (unsigned int y = 0; y < m_height; ++y)
		{
			for (unsigned int x = 0; x < m_width; ++x)
			{
				SetColor(x, y, color);
			}
		}
	}

private:
	Color* m_buffer; // buffer 从屏幕最下面一行像素开始网上
	unsigned int m_width;
	unsigned int m_height;
};