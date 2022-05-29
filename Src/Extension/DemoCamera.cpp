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
#define input Input::Instance()

static const float DEFAULT_MOVE_SPEED = 1.0f;
static const float DEFAULT_MOUSE_SENSITIVITY = 1.0f;

static const float DEFAULT_FOV_SPAN = 0.1f;
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
	m_onInput = { std::bind(&DemoCamera::OnUpdate, this) };
	m_onMouseMove = { std::bind(&DemoCamera::OnMouseMove, this, std::placeholders::_1) };
	m_onMouseScroll = { std::bind(&DemoCamera::OnMouseScroll, this, std::placeholders::_1) };

	app.InputEvent += m_onInput;
	
	input.MouseMovedEvent += m_onMouseMove;
	input.MouseScrolledEvent += m_onMouseScroll;
}

DemoCamera::~DemoCamera()
{
	app.InputEvent -= m_onInput;

	input.MouseMovedEvent -= m_onMouseMove;
	input.MouseScrolledEvent -= m_onMouseScroll;
}

void DemoCamera::OnUpdate()
{
	MovementControll();
	return;
	if (IsPerspective()) PerspectiveParamControll();
	else /*IsOrthographic*/ OrthgraphicSizeControll();
}

void DemoCamera::MovementControll()
{
	float x = 0.0f, y = 0.f;
	if (input.GetKey(UP_KEY))    y += 1.0f;
	if (input.GetKey(DOWN_KEY))  y -= 1.0f;
	if (input.GetKey(LEFT_KEY))  x -= 1.0f;
	if (input.GetKey(RIGHT_KEY)) x += 1.0f;
	vec3 dir = { x, y, 0.0f };
	dir = glm::normalize(dir);
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

}

void DemoCamera::OnMouseScroll(int offset)
{
	return;
	if (IsPerspective())
	{
		float newFov = GetFov() + m_fovSpan * static_cast<float>(offset);
		SetFov(newFov);
	}
	else
	{
		vec2 newZSize = ChangeSize(GetZSize(), m_zSizeSpan * static_cast<float>(offset));
		SetZSize(newZSize);
	}
}