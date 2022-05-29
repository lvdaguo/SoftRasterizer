#pragma once

class Input : public Singleton<Input>
{
public:
	Input() { TIPS(L"Input Con"); }
	void Init();
	~Input();

public:
	bool GetKey(short int key) const { return GetKeyState(key) & 0x8000; }
	vec2i GetMousePos() const { return m_mousePos; }
	int GetMouseWheel() const { return m_wheelOffset; }

public:
	Event1<int> MouseScrolledEvent;
	Event1<vec2i> MouseMovedEvent;

private:
	void OnMsgReceived(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	Action4<HWND, UINT, WPARAM, LPARAM> m_onMsgReceived;
	vec2i m_mousePos;
	int m_wheelOffset;
};
