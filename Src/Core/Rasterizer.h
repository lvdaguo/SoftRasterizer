#pragma once

class Rasterizer : public Singleton<Rasterizer>
{
public:
	Rasterizer() { }
	virtual ~Rasterizer() override;

	void Init();

	void DrawTriangle();
	void DrawBuffer();

	void SetClearColor(struct Color color);

private:
	void OnUpdate();

	struct Color m_clearColor;

	class ColorBuffer* m_colorBuffer;
	Action m_onUpdate;

	HDC m_hDC, m_hMem;
	unsigned int m_width, m_height;
};