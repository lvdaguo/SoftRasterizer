#pragma once

#include "Core/Window.h"
#include "Core/MatrixTool.h"

class Camera
{
public:
	Camera(vec3 pos, vec3 front);

// matrix
public:
	mat4 GetViewProjection() { TryUpdateViewProjection(); return m_viewProjection; }
	mat4 GetView() { TryUpdateView(); return m_view; }
	mat4 GetProjection() { TryUpdateProjection(); return m_projection; }

private:
	void TryUpdateViewProjection() 
	{ 
		if (m_viewNeedUpdate == false && m_projectionNeedUpdate == false) return;
		TryUpdateView();
		TryUpdateProjection();
		m_viewProjection = m_projection * m_view;
	}

	void TryUpdateView()
	{
		if (m_viewNeedUpdate == false) return;
		m_view = MatrixTool::look_at(m_position, m_position + m_front, m_up);
		m_viewNeedUpdate = false;
	}

	void TryUpdateProjection() 
	{ 
		if (m_projectionNeedUpdate == false) return;
		if (m_isPerspective)
			m_projection = MatrixTool::perspective(m_fov, Window::Instance().GetAspect(), m_near, m_far);
		else // orthographic
			m_projection = MatrixTool::ortho(m_xSize.x, m_xSize.y, m_ySize.x, m_ySize.y, m_zSize.x, m_zSize.y);
		m_projectionNeedUpdate = false;
	}

private:
	mat4 m_view, m_projection, m_viewProjection;
	bool m_viewNeedUpdate, m_projectionNeedUpdate; // 数据脏标记

// perspective specific
public:
	void SetFov(float fov) { m_fov = fov; m_projectionNeedUpdate = true; }
	void SetNearPlane(float nearZ) { m_near = nearZ; m_projectionNeedUpdate = true; }
	void SetFarPlane(float farZ) { m_far = farZ; m_projectionNeedUpdate = true; }

private:
	float m_fov;
	float m_near, m_far;

// orthographic specific
public: 
	void SetXSize(vec2 xSize) { m_xSize = xSize; m_projectionNeedUpdate = true; }
	void SetYSize(vec2 ySize) { m_ySize = ySize; m_projectionNeedUpdate = true; }
	void SetZSize(vec2 zSize) { m_zSize = zSize; m_projectionNeedUpdate = true; }

private:
	vec2 m_xSize, m_ySize, m_zSize;

// member
public:
	void SetProjectionMode(bool isPerspective) { m_isPerspective = isPerspective; m_projectionNeedUpdate = true; }
	void SetPosition(vec3 pos) { m_position = pos; m_viewNeedUpdate = true; }
	void SetFront(vec3 dir) { m_front = dir; m_viewNeedUpdate = true; }
	void SetUp(vec3 up) { m_up = up; m_viewNeedUpdate = true; }

public:
	vec3 GetPosition() const { return m_position; }

private:
	bool m_isPerspective;
	vec3 m_position;
	vec3 m_front, m_up;
};

