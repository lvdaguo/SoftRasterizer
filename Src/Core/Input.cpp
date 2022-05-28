#include "pch.h"
#include "Input.h"

#include "Core/Window.h"
#include <windowsx.h>

static Window& window = Window::Instance();
static Input& self = Input::Instance();

void Input::Init()
{
	m_onMsgReceived = { std::bind(Input::OnMsgReceived, 
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4) };
	window.MsgReceivedEvent += m_onMsgReceived;
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
		if (delta != self.m_wheelOffset)
		{
			self.MouseWheelRolledEvent.Invoke(delta - self.m_wheelOffset);
			self.m_wheelOffset = delta;
		}
		break;
	case WM_MOUSEMOVE:
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		if (x != self.m_mousePos.x || y != self.m_mousePos.y)
		{
			self.MouseMovedEvent.Invoke({ x - self.m_mousePos.x, y - self.m_mousePos.y });
			self.m_mousePos = { x, y };
		}
		break;
	}
}
