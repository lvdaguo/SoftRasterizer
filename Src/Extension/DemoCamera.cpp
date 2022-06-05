#include "pch.h"
#include "DemoCamera.h"

#include "Core/Application.h"
#include "Core/Input.h"

#define UP_KEY    'W'
#define DOWN_KEY  'S'
#define LEFT_KEY  'A'
#define RIGHT_KEY 'D'

#define NEAR_DEC '7'
#define NEAR_INC '8'

#define FAR_DEC '9'
#define FAR_INC '0'

#define X_SIZE_DEC '7'
#define X_SIZE_INC '8'

#define Y_SIZE_DEC '9'
#define Y_SIZE_INC '0'

#define app Application::Instance()
#define window Window::Instance()
#define input Input::Instance()

static const float DEFAULT_MOVE_SPEED = 3.0f;
static const float DEFAULT_MOUSE_SENSITIVITY = 0.001f;

static const float DEFAULT_FOV_SPAN = 0.01f;
static const float DEFAULT_NEAR_SPAN = 0.1f;
static const float DEFAULT_FAR_SPAN = 1.0f;

static const float DEFAULT_X_SIZE_SPAN = 0.5f;
static const float DEFAULT_Y_SIZE_SPAN = 0.5f;
static const float DEFAULT_Z_SIZE_SPAN = 0.5f;

DemoCamera::DemoCamera(vec3 pos, vec3 front) : Camera(pos, front),
	m_moveSpeed(DEFAULT_MOVE_SPEED), m_mouseSensitivity(DEFAULT_MOUSE_SENSITIVITY),
	m_fovSpan(DEFAULT_FOV_SPAN),
	m_nearSpan(DEFAULT_NEAR_SPAN), m_farSpan(DEFAULT_FAR_SPAN),
	m_xSizeSpan(DEFAULT_X_SIZE_SPAN) , m_ySizeSpan(DEFAULT_Y_SIZE_SPAN), m_zSizeSpan(DEFAULT_Z_SIZE_SPAN)
{
	m_onInput = { std::bind(&DemoCamera::OnInput, this) };
	m_onMouseMove = { std::bind(&DemoCamera::OnMouseMove, this, std::placeholders::_1) };
	m_onMouseScroll = { std::bind(&DemoCamera::OnMouseScroll, this, std::placeholders::_1) };

	app.InputEvent += m_onInput;
	
	input.MouseMovedEvent += m_onMouseMove;
	input.MouseScrolledEvent += m_onMouseScroll;

	ShowCursor(FALSE);
}

DemoCamera::~DemoCamera()
{
	app.InputEvent -= m_onInput;

	input.MouseMovedEvent -= m_onMouseMove;
	input.MouseScrolledEvent -= m_onMouseScroll;
}

void DemoCamera::OnInput()
{
	if (window.GetHWND() == GetActiveWindow()) // 当前窗口为活跃状态
	{
		// 将光标锁定在屏幕中心，使用Raw Input获取鼠标输入
		WINDOWPLACEMENT wp;
		GetWindowPlacement(window.GetHWND(), &wp);
		RECT& rect = wp.rcNormalPosition;
		int x = (rect.left + rect.right) / 2;
		int y = (rect.bottom + rect.top) / 2;

		static RECT bound;
		bound.left = bound.right = x;
		bound.bottom = bound.top = y;
		ClipCursor(&bound);
	}
	else // 当前窗口非活跃状态
	{
		ClipCursor(NULL);
	}

	MovementControll();
	if (IsPerspective()) PerspectiveParamControll();
	else /*IsOrthographic*/ OrthgraphicSizeControll();
}

void DemoCamera::MovementControll()
{
	float right = 0.0f, front = 0.0f;
	if (input.GetKey(UP_KEY))    front += 1.0f;
	if (input.GetKey(DOWN_KEY))  front -= 1.0f;
	if (input.GetKey(LEFT_KEY))  right -= 1.0f;
	if (input.GetKey(RIGHT_KEY)) right += 1.0f;

	vec3 dir = GetFront() * front + GetRight() * right;
	float len = glm::length(dir);
	if (len != 0.0f) dir /= len;
	vec3 newPos = GetPosition() + dir * m_moveSpeed * app.GetDeltaTime();
	SetPosition(newPos);
}

void DemoCamera::PerspectiveParamControll()
{
	if (input.GetKey(NEAR_DEC))	SetNear(GetNear() - m_nearSpan * app.GetDeltaTime());
	if (input.GetKey(NEAR_INC))	SetNear(GetNear() + m_nearSpan * app.GetDeltaTime());
	if (input.GetKey(FAR_DEC)) SetFar(GetFar() - m_farSpan * app.GetDeltaTime());
	if (input.GetKey(FAR_INC)) SetFar(GetFar() + m_farSpan * app.GetDeltaTime());
}

vec2 DemoCamera::ChangeSize(vec2 ori, float amount)
{
	float center = (ori.x + ori.y) / 2.0f;
	float size = glm::abs(ori.y - center);
	size += amount;
	return { center - size, center + size };
}

void DemoCamera::OrthgraphicSizeControll()
{
	if (input.GetKey(X_SIZE_DEC))
	{
		vec2 newXSize = ChangeSize(GetXSize(), m_xSizeSpan * app.GetDeltaTime() * -1.0f);
		SetXSize(newXSize);
	}
	if (input.GetKey(X_SIZE_INC))
	{
		vec2 newXSize = ChangeSize(GetXSize(), m_xSizeSpan * app.GetDeltaTime());
		SetXSize(newXSize);
	}
	if (input.GetKey(Y_SIZE_DEC))
	{
		vec2 newYSize = ChangeSize(GetYSize(), m_ySizeSpan * app.GetDeltaTime() * -1.0f);
		SetYSize(newYSize);
	}
	if (input.GetKey(Y_SIZE_INC))
	{
		vec2 newYSize = ChangeSize(GetYSize(), m_ySizeSpan * app.GetDeltaTime());
		SetYSize(newYSize);
	}
}

void DemoCamera::OnMouseMove(vec2i offset)
{
	float x = offset.x * m_mouseSensitivity;
	float y = offset.y * m_mouseSensitivity;

	static const vec3 xAxis = { 1.0f, 0.0f, 0.0f };
	static const vec3 yAxis = { 0.0f, 1.0f, 0.0f };

	mat3 ro1 = MatrixLib::Rotate(-x, yAxis);
	mat3 ro2 = MatrixLib::Rotate(-y, xAxis);
	vec3 newDir = ro2 * ro1 * GetFront();
	SetFront(newDir);
}

void DemoCamera::OnMouseScroll(int offset)
{
	if (IsPerspective())
	{
		float newFov = GetFov() + glm::radians(m_fovSpan * static_cast<float>(-offset));
		SetFov(newFov);
	}
	else
	{
		vec2 newZSize = ChangeSize(GetZSize(), m_zSizeSpan * static_cast<float>(offset));
		SetZSize(newZSize);
	}
}