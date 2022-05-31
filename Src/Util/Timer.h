#pragma once

#include "pch.h"

// RAII¼ÆÊ±Æ÷
class Timer
{
public:
	Timer(float* duration) : m_duration(duration)
	{
		m_start = std::chrono::steady_clock::now();
	}

	~Timer()
	{
		auto end = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();
		*m_duration = static_cast<float>(duration * 0.000001);
	}

private:
	float* m_duration;
	std::chrono::time_point<std::chrono::steady_clock> m_start;
};