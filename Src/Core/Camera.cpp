#include "pch.h"
#include "Camera.h"

static const bool default_is_perspective = true;
static const vec3 world_up = { 0.0f, 1.0f, 0.0f };

static const float default_near_plane = 0.1f;
static const float default_far_plane = 50.0f;
static const float default_fov = glm::radians(45.0f);

static const vec2 default_x_size = { -5.0f, 5.0f };
static const vec2 default_y_size = { -5.0f, 5.0f };
static const vec2 default_z_size = { 0.0f, 20.0f };

Camera::Camera(vec3 pos, vec3 front) : m_position(pos), m_front(front),
	m_viewNeedUpdate(true), m_projectionNeedUpdate(true),
	m_up(world_up), m_isPerspective(default_is_perspective),
	m_fov(default_fov), m_near(default_near_plane), m_far(default_far_plane), 
	m_xSize(default_x_size), m_ySize(default_y_size), m_zSize(default_z_size)
{ }

