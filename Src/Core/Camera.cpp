#include "pch.h"
#include "Camera.h"

static const bool DEFAULT_IS_PERSPECTIVE = true;
static const vec3 WORLD_UP = { 0.0f, 1.0f, 0.0f };

static const float DEFAULT_NEAR_PLANE = 0.1f;
static const float DEFAULT_FAR_PLANE = 50.0f;
static const float DEFAULT_FOV = glm::radians(45.0f);

static const vec2 DEFAULT_X_SIZE = { -5.0f, 5.0f };
static const vec2 DEFAULT_Y_SIZE = { -5.0f, 5.0f };
static const vec2 DEFAULT_Z_SIZE = { 0.0f, 20.0f };

Camera::Camera(const vec3& pos, const vec3& front) 
	: m_position(pos), m_front(front),
	m_view(), m_projection(), m_viewProjection(),
	m_viewNeedUpdate(true), m_projectionNeedUpdate(true),

	m_worldUp(WORLD_UP), m_isPerspective(DEFAULT_IS_PERSPECTIVE),
	
	m_fov(DEFAULT_FOV), m_near(DEFAULT_NEAR_PLANE), m_far(DEFAULT_FAR_PLANE), 
	m_xSize(DEFAULT_X_SIZE), m_ySize(DEFAULT_Y_SIZE), m_zSize(DEFAULT_Z_SIZE)
{ }

