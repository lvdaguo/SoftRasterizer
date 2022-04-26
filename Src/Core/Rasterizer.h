#pragma once

class Rasterizer : public Singleton<Rasterizer>
{
public:
	Rasterizer();

	void DrawTriangle();
	void DrawBuffer();
};