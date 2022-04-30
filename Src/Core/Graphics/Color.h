#pragma once

#include "pch.h"

struct Color
{
	// 由于实际显示的是BGRA，所以换一下顺序，让其符合RGBA
	byte b;
	byte g;
	byte r;
	byte a;

	Color(byte r = 255, byte g = 255, byte b = 255, byte a = 255)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
};