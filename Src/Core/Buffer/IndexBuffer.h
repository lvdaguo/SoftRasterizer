#pragma once

#include "pch.h"

class IndexBuffer
{
public:
	IndexBuffer(unsigned int* buffer, unsigned int count)
		: m_buffer(buffer), m_count(count)
	{ }

public:
	unsigned int operator[](unsigned int i) const
	{
		return m_buffer[i];
	}

	unsigned int GetCount() const { return m_count; }

	unsigned int* begin() { return m_buffer; }
	unsigned int* end() { return m_buffer + m_count; }
	const unsigned int* begin() const { return m_buffer; }
	const unsigned int* end() const { return m_buffer + m_count; }

private:
	unsigned int* m_buffer;
	unsigned int m_count;
};