#pragma once

class Window : public Singleton<Window>
{
public:
	Window() : m_hWnd(nullptr), m_width(0), m_height(0) { }

	HWND GetHWND() const { return m_hWnd; }
	unsigned int GetWidth() const { return m_width; }
	unsigned int GetHeight() const { return m_height; }
	float GetAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

	void Init(HINSTANCE hInstance, unsigned int width, unsigned int height, class Application& app);

private:
	HWND m_hWnd;
	unsigned int m_width, m_height;
};

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);