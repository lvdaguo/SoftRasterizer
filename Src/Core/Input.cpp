#include "pch.h"
#include "Input.h"

#include "Core/Application.h"
#include "Core/Window.h"
#include <windowsx.h>

#define app Application::Instance()
#define window Window::Instance()

static const unsigned int VKEY_RANGE = 256;

void Input::Init()
{
	m_onMsgReceived = { std::bind(&Input::OnMsgReceived, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4) };
	
	m_onInternalInput = { std::bind(&Input::OnInternalInput, this) };
	m_onInput = { std::bind(&Input::OnInput, this) };
	
	window.MsgReceivedEvent += m_onMsgReceived;
	app.InternalInputEvent += m_onInternalInput;
	app.InputEvent += m_onInput;

	LOG_TRACE("init input");

	// 创建原始输入设备
	static RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = ((USHORT)0x01);
	Rid[0].usUsage = ((USHORT)0x02);
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = window.GetHWND();
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	m_keyStateMap = std::vector<bool>(VKEY_RANGE, false);
	m_lastKeyStateMap = std::vector<bool>(VKEY_RANGE, false);
}

Input::~Input()
{
	app.InputEvent -= m_onInput;
	app.InternalInputEvent -= m_onInternalInput;
	window.MsgReceivedEvent -= m_onMsgReceived;
}

void Input::OnMsgReceived(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int delta, x, y;
	switch (msg)
	{
	case WM_MOUSEWHEEL:
		delta = GET_WHEEL_DELTA_WPARAM(wParam);
		m_mouseScrolledBuffer.push(delta); // 将事件缓冲，到OnInput阶段一次性全部发出
		break;
	case WM_INPUT:
		UINT dwSize;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			if ((raw->data.mouse.usFlags & MOUSE_MOVE_RELATIVE) == MOUSE_MOVE_RELATIVE)
			{
				LONG x = raw->data.mouse.lLastX;
				LONG y = raw->data.mouse.lLastY;
				m_mouseMovedBuffer.push({ x, y });
			}
		}
		break;
	}
}

void Input::OnInternalInput()
{
	std::swap(m_lastKeyStateMap, m_keyStateMap);
	for (int i = 0; i < (int)VKEY_RANGE; ++i) // 更新所有key的map
	{
		m_keyStateMap[i] = (GetKeyState(i) & 0x8000);
	}
}

void Input::OnInput()
{
	while (m_mouseMovedBuffer.size())
	{
		MouseMovedEvent.Invoke(m_mouseMovedBuffer.front());
		m_mouseMovedBuffer.pop();
	}

	while (m_mouseScrolledBuffer.size())
	{
		MouseScrolledEvent.Invoke(m_mouseScrolledBuffer.front());
		m_mouseScrolledBuffer.pop();
	}
}