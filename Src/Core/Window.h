#pragma once

class Window : public Singleton<Window>
{
public:
	Window() { }

	void Init(HINSTANCE hInstance, unsigned int width, unsigned int height, class Application& app);
};

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);