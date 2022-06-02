#pragma once

#include "Core/Window.h"
#include "Core/MatrixTool.h"

class Camera
{
public:
	Camera(const vec3& pos, const vec3& front);

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
		if (m_front == m_up || m_front == -m_up) { TIPS(L"相机方向和世界的上方向共线，两向量叉积结果为0向量，出错"); }
		m_view = MatrixTool::LookAt(m_position, m_position + m_front, m_up);
		m_viewNeedUpdate = false;
	}

	void TryUpdateProjection() 
	{ 
		if (m_projectionNeedUpdate == false) return;
		if (m_isPerspective)
			m_projection = MatrixTool::Perspective(m_fov, Window::Instance().GetAspect(), m_near, m_far);
		else // orthographic
			m_projection = MatrixTool::Ortho(m_xSize.x, m_xSize.y, m_ySize.x, m_ySize.y, m_zSize.x, m_zSize.y);
		m_projectionNeedUpdate = false;
	}

private:
	mat4 m_view, m_projection, m_viewProjection;
	bool m_viewNeedUpdate, m_projectionNeedUpdate; // 数据脏标记

// perspective specific
public:
	float GetFov() const { return m_fov; }
	float GetNear() const { return m_near; }
	float GetFar() const { return m_far; }

public:
	void SetFov(float fov) { m_fov = fov; m_projectionNeedUpdate = true; }
	void SetNear(float nearZ) { m_near = nearZ; m_projectionNeedUpdate = true; }
	void SetFar(float farZ) { m_far = farZ; m_projectionNeedUpdate = true; }

private:
	float m_fov;
	float m_near, m_far;

// orthographic specific
public:
	vec2 GetXSize() const { return m_xSize; }
	vec2 GetYSize() const { return m_ySize; }
	vec2 GetZSize() const { return m_zSize; }

public: 
	void SetXSize(const vec2& xSize) { m_xSize = xSize; m_projectionNeedUpdate = true; }
	void SetYSize(const vec2& ySize) { m_ySize = ySize; m_projectionNeedUpdate = true; }
	void SetZSize(const vec2& zSize) { m_zSize = zSize; m_projectionNeedUpdate = true; }

private:
	vec2 m_xSize, m_ySize, m_zSize;

// member
public:
	bool IsPerspective() const { return m_isPerspective; }
	vec3 GetPosition() const { return m_position; }
	vec3 GetFront() const { return m_front; }
	vec3 GetUp() const { return m_up; }

public:
	void SetProjectionMode(bool isPerspective) { m_isPerspective = isPerspective; m_projectionNeedUpdate = true; }
	void SetPosition(const vec3& pos) { m_position = pos; m_viewNeedUpdate = true; }
	void SetFront(const vec3& dir) { if (m_front != dir) { m_front = dir; m_viewNeedUpdate = true; } }
	void SetUp(const vec3& up) { m_up = up; m_viewNeedUpdate = true; }

private:
	bool m_isPerspective;
	vec3 m_position;
	vec3 m_front, m_up;
};

