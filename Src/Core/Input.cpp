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
		if (delta != m_wheelOffset)
		{
			MouseScrolledEvent.Invoke(delta - m_wheelOffset);
			m_wheelOffset = delta;
		}
		break;
	case WM_MOUSEMOVE:
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		if (x != m_mousePos.x || y != m_mousePos.y)
		{
			MouseMovedEvent.Invoke({ x - m_mousePos.x, y - m_mousePos.y });
			m_mousePos = { x, y };
		}
		break;
	}
}
