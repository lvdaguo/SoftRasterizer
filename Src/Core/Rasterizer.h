#pragma once

struct Color {
	// ����ʵ����ʾ����BGRA�����Ի�һ��˳���������RGBA
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
			//����Ļ���Ϊ������32λλͼ��һ�����صĴ�СΪ4�ֽڣ�������Ϊ����λͼ��ռ���ֽ���
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