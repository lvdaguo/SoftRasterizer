#pragma once

#include "pch.h"

class DepthBuffer
{
public:
	DepthBuffer(unsigned int width, unsigned int height, unsigned int subsampleCount = 1) : 
		m_width(width), m_height(height), 
		m_subsampleCount(subsampleCount),
		m_buffer(width * height * subsampleCount, std::numeric_limits<float>::max()) // ����ռ䲢Clear
	{ }

	~DepthBuffer() { }

public:
	void SetSubsampleCount(unsigned int subSampleCount)
	{
		m_subsampleCount = subSampleCount;
		m_buffer.resize(m_width * m_height * m_subsampleCount);
		Clear();
	}

	float GetDepth(unsigned int x, unsigned int y, unsigned int subsampleIndex = 0)
	{
		// yֵ�����ڵڼ��У�xֵ�����ڵڼ���
		return m_buffer[(y * m_width + x) * m_subsampleCount + subsampleIndex];
	}

	void SetDepth(unsigned int x, unsigned int y, float depth, unsigned int subsampleIndex = 0)
	{
		// yֵ�����ڵڼ��У�xֵ�����ڵڼ���
		m_buffer[(y * m_width + x) * m_subsampleCount + subsampleIndex] = depth;
	}

	void Clear()
	{
		// һ��һ��
		for (unsigned int i = 0; i < m_width * m_height * m_subsampleCount; ++i)
		{
			m_buffer[i] = std::numeric_limits<float>::max();
		}
	}

private:
	std::vector<float> m_buffer;
	unsigned int m_width, m_height;
	unsigned int m_subsampleCount;
};