#pragma once

#include "Core/Camera.h"

class DemoCamera : public Camera
{
public:
	DemoCamera(vec3 pos, vec3 front);
	~DemoCamera();

private:
	void OnUpdate();

	void MovementControll();
	void PerspectiveParamControll();
	void OrthgraphicSizeControll();

private:
	vec2 ChangeSize(vec2 ori, float amount);

private:
	void OnMouseMove(vec2i offset);
	void OnMouseScroll(int offset);

private:
	Action m_onInput;
	Action1<vec2i> m_onMouseMove;
	Action1<int> m_onMouseScroll;

	float m_moveSpeed;
	float m_mouseSensitivity;
	
	// perspective
	float m_fovSpan;
	float m_nearSpan;
	float m_farSpan;

	// orthographic
	float m_xSizeSpan, m_ySizeSpan, m_zSizeSpan;
};

