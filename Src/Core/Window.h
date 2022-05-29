#pragma once

class Window : public Singleton<Window>
{
public:
	void Init(HINSTANCE hInstance, unsigned int width, unsigned int height, class Application& app);

public:
	HWND GetHWND() const { return m_hWnd; }
	unsigned int GetWidth() const { return m_width; }
	unsigned int GetHeight() const { return m_height; }
	float GetAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

	Event4<HWND, UINT, WPARAM, LPARAM> MsgReceivedEvent;

private:
	HWND m_hWnd;
	unsigned int m_width, m_height;
};

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
