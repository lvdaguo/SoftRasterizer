#pragma once

struct Color {
	// 由于实际显示的是BGRA，所以换一下顺序，让其符合RGBA
	byte m_b;
	byte m_g;
	byte m_r;
	byte m_a;

	Color(byte r = 255, byte g = 255, byte b = 255, byte a = 255) 
	{
		m_r = r;
		m_g = g;
		m_b = b;
		m_a = a;
	}
};

struct Point 
{
	float m_x;
	float m_y;
	Color m_color;

	Point(float x, float y, Color color) {
		m_x = x;
		m_y = y;
		m_color = color;
	}
};

class Canvas 
{
public:
	unsigned int m_width;
	unsigned int m_height;
	Color* m_buffer;

public:
	Canvas(unsigned int width, unsigned int height, Color* color)
	{
		m_width = width;
		m_height = height;
		m_buffer = color;
	}

	~Canvas()
	{
		delete m_buffer;
	}

	void ClearColorBuffer()
	{
		if (m_buffer != nullptr) 
		{
			//将屏幕清空为黑屏，32位位图的一个像素的大小为4字节，第三项为整个位图所占的字节数
			memset(m_buffer, 0, m_width * m_height * sizeof(Color));
		}
	}
};

class Rasterizer : public Singleton<Rasterizer>
{
public:
	Rasterizer() { }
	virtual ~Rasterizer() override;

	void Init();

	void DrawTriangle();
	void DrawBuffer();

private:
	void OnUpdate();

	Canvas* m_canvas;
	Action m_onUpdate;

	HDC m_hDC, m_hMem;
	unsigned int m_width, m_height;
};