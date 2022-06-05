#pragma once

class Input : public Singleton<Input>
{
public:
	void Init();
	~Input();

public:
	bool GetKey(int key) const { return m_keyStateMap[key]; }
	bool GetKeyDown(int key) const { return m_lastKeyStateMap[key] == false && m_keyStateMap[key]; }
	bool GetKeyUp(int key) const { return m_lastKeyStateMap[key] && m_keyStateMap[key] == false; }

public:
	Event1<int> MouseScrolledEvent;
	Event1<vec2i> MouseMovedEvent;

	std::queue<int> m_mouseScrolledBuffer;
	std::queue<vec2i> m_mouseMovedBuffer;

private:
	void OnMsgReceived(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void OnInternalInput();
	void OnInput();

private:
	Action4<HWND, UINT, WPARAM, LPARAM> m_onMsgReceived;
	Action m_onInternalInput, m_onInput;

	std::vector<bool> m_keyStateMap, m_lastKeyStateMap;
};

