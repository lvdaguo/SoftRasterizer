#pragma once

#include "pch.h"
#include "params.h"

class MatrixTool
{
	static mat4 translate(vec3& pos)
	{
		if (USING_GLM_MATRIX) return glm::translate(mat4(1.0f), pos);

		return mat4
		(
			1.f, 0.f, 0.f, pos.x,
			0.f, 1.f, 0.f, pos.y,
			0.f, 0.f, 1.f, pos.z,
			0.f, 0.f, 0.f, 1.f
		);
	}

	static mat4 rotate(float angle, vec3& axis)
	{
		if (USING_GLM_MATRIX) return glm::rotate(mat4(1.0f), angle, axis);

		const float x = axis.x, y = axis.y, z = axis.z;
		const float si = sin(deg2rad(angle)), co = cos(deg2rad(angle));
		return mat4
		(
			co+(1.f-co)*x*x,    -si*z+(1.f-co)*x*y, si*y+(1.f-co)*x*z,  0.f,
			si*z+(1.f-co)*x*y,  co+(1.f-co)*y*y,    -si*x+(1.f-co)*y*z, 0.f,
			-si*y+(1.f-co)*x*z, si*x+(1.f-co)*y*z,  co+(1.f-co)*z*z,    0.f,
			0.f, 0.f, 0.f, 1.f
		);
	}

	static mat4 scale(vec3& scale)
	{
		if (USING_GLM_MATRIX) return glm::scale(mat4(1.0f), scale);

		return mat4
		(
			scale.x, 0.f,     0.f, 0.f,
			0.f,	 scale.y, 0.f, 0.f,
			0.f,	 0.f,     scale.z, 0.f,
			0.f,	 0.f,	  0.f, 1.f
		);
	}

	// 摄影机变换矩阵：eye/视点位置，at/看向哪里，up/指向上方的矢量
	static mat4 look_at(vec3& eyePos, vec3& at, vec3& up)
	{
		if (USING_GLM_MATRIX) return glm::lookAt(eyePos, at, up);

	}

	static mat4 ortho(float l, float r, float b, float t, float n, float f)
	{
		if (USING_GLM_MATRIX) return glm::ortho(l, r, b, t, n, f);

		vec3 pos = vec3(-(l + r) / 2, -(b + t) / 2, -(f + n) / 2);
		vec3 size = vec3(2 / (r - l), 2 / (t - b), 2 / (n - f));
		mat4 ortho_translation = translate(pos);
		mat4 ortho_scale = scale(size);
		return ortho_scale * ortho_translation;
	}

	static mat4 perspective(float fovy, float aspect, float n, float f)
	{
		if (USING_GLM_MATRIX) return glm::perspective(fovy, aspect, n, f);

		const float half_fov_deg = fovy * 0.5f, half_fov_rad = deg2rad(half_fov_deg);
		const float t = abs(n * tan(half_fov_rad)), b = -t;
		const float r = abs(t * aspect), l = -r;

		mat4 persp = mat4
		(
			n,    0.f,  0.f,    0.f,
			0.f,  n,    0.f,    0.f,
			0.f,  0.f,  n + f, -n * f,
			0.f,  0.f,  1.f,    0.f
		);
		return ortho(l, r, b, t, n, f) * persp;
	}

};