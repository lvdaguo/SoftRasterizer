#pragma once

class Random
{
public:
	static int Range(int l, int r)
	{
		std::uniform_int_distribution<int> uid(l, r);
		return uid(rng);
	}

	static unsigned int Range(unsigned int l, unsigned int r)
	{
		std::uniform_int_distribution<unsigned int> uid(l, r);
		return uid(rng);
	}

private:
	static std::random_device rd;
	static std::mt19937 rng;
};