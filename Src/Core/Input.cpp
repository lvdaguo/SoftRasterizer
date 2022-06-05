#include "pch.h"
#include "Input.h"

#include "Core/Window.h"
#include <windowsx.h>

#define window Window::Instance()

void Input::Init()
{
	m_onMsgReceived = { std::bind(&Input::OnMsgReceived, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4) };
	window.MsgReceivedEvent += m_onMsgReceived;

	LOG_TRACE("init input");

	static RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = ((USHORT)0x01);
	Rid[0].usUsage = ((USHORT)0x02);
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = window.GetHWND();
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
}

Input::~Input()
{
	window.MsgReceivedEvent -= m_onMsgReceived;
}

void Input::OnMsgReceived(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int delta, x, y;
	switch (msg)
	{
	case WM_MOUSEWHEEL:
		delta = GET_WHEEL_DELTA_WPARAM(wParam);
		MouseScrolledEvent.Invoke(delta);
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
				MouseMovedEvent.Invoke({ x, y });
			}
		}
		break;
	}
}
