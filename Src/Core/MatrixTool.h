#pragma once

#include "pch.h"
#include "params.h"

// 默认为 -1 1
// 右手系

// #define GLM_CLIP_CONTROL_ZO_BIT		(1 << 0) // ZERO_TO_ONE
// #define GLM_CLIP_CONTROL_NO_BIT		(1 << 1) // NEGATIVE_ONE_TO_ONE
// #define GLM_CLIP_CONTROL_LH_BIT		(1 << 2) // LEFT_HANDED, For DirectX, Metal, Vulkan
// #define GLM_CLIP_CONTROL_RH_BIT		(1 << 3) // RIGHT_HANDED, For OpenGL, default in GLM

// #define GLM_CLIP_CONTROL_RH_NO (GLM_CLIP_CONTROL_RH_BIT | GLM_CLIP_CONTROL_NO_BIT)

// #define GLM_CONFIG_CLIP_CONTROL GLM_CLIP_CONTROL_RH_NO

// mat4的内存布局为按列存储，因为opengl在显存中也是按列存储的
// mat[0]为第一列，mat[2][3]为第三列第四个元素

class MatrixTool
{
public:
	
	static mat4 translate(const vec3& pos)
	{
		if (USING_GLM_MATRIX) return glm::translate(mat4(1.0f), pos);

		mat4 res;
		res[0][0] = 1.f, res[1][0] = 0.f, res[2][0] = 0.f, res[3][0] = pos.x;
		res[0][1] = 0.f, res[1][1] = 1.f, res[2][1] = 0.f, res[3][1] = pos.y;
		res[0][2] = 0.f, res[1][2] = 0.f, res[2][2] = 1.f, res[3][2] = pos.z;
		res[0][3] = 0.f, res[1][3] = 0.f, res[2][3] = 0.f, res[3][3] = 1.f;
		return res;
	}

	static mat4 rotate(float angle, const vec3& axis)
	{
		if (USING_GLM_MATRIX) return glm::rotate(mat4(1.0f), angle, axis);

		vec3 n = glm::normalize(axis);
		const float x = n.x, y = n.y, z = n.z;
		const float si = glm::sin(angle), co = glm::cos(angle);

		mat4 res;
		res[0][0] = co+(1.f-co)*x*x,    res[1][0] = -si*z+(1.f-co)*x*y,  res[2][0] = si*y+(1.f-co)*x*z,  res[3][0] = 0.f;
		res[0][1] = si*z+(1.f-co)*x*y,  res[1][1] = co+(1.f-co)*y*y,     res[2][1] = -si*x+(1.f-co)*y*z, res[3][1] = 0.f;
		res[0][2] = -si*y+(1.f-co)*x*z, res[1][2] = si*x + (1.f-co)*y*z, res[2][2] = co+(1.f-co)*z*z,    res[3][2] = 0.f;
		res[0][3] = 0.f,                res[1][3] = 0.f,                 res[2][3] = 0.f,                res[3][3] = 1.f;
		
		return res;
	}

	static mat4 scale(const vec3& scale)
	{
		if (USING_GLM_MATRIX) return glm::scale(mat4(1.0f), scale);

		mat4 res;
		res[0][0] = scale.x, res[1][0] = 0.f,     res[2][0] = 0.f,     res[3][0] = 0.f;
		res[0][1] = 0.f,     res[1][1] = scale.y, res[2][1] = 0.f,     res[3][1] = 0.f;
		res[0][2] = 0.f,     res[1][2] = 0.f,     res[2][2] = scale.z, res[3][2] = 0.f;
		res[0][3] = 0.f,     res[1][3] = 0.f,     res[2][3] = 0.f,     res[3][3] = 1.f;

		return res;
	}

	// 摄影机变换矩阵：eye/视点位置，at/看向哪里，up/世界的上方向
	static mat4 look_at(const vec3& eyePos, const vec3& at, const vec3& up)
	{
		if (USING_GLM_MATRIX) return glm::lookAt(eyePos, at, up); 

		vec3 f = glm::normalize(at - eyePos);
		vec3 r = glm::normalize(glm::cross(f, up));
		vec3 u = glm::normalize(glm::cross(r, f));

		mat4 rotation;
		rotation[0][0] =  r.x, rotation[1][0] =  r.y, rotation[2][0] =  r.z, rotation[3][0] = 0.f;
		rotation[0][1] =  u.x, rotation[1][1] =  u.y, rotation[2][1] =  u.z, rotation[3][1] = 0.f;
		rotation[0][2] = -f.x, rotation[1][2] = -f.y, rotation[2][2] = -f.z, rotation[3][2] = 0.f;
		rotation[0][3] =  0.f, rotation[1][3] =  0.f, rotation[2][3] =  0.f, rotation[3][3] = 1.f;

		mat4 res = rotation * translate(-eyePos);
		return res;
	}

	// 让 n 小于 f 即可，与坐标轴是否为右手系无关，传入的参数无需考虑此类细节
	static mat4 ortho(float l, float r, float b, float t, float n, float f)
	{
		if (USING_GLM_MATRIX) return glm::ortho(l, r, b, t, n, f);

		mat4 res;
		res[0][0] = 2.f/(r-l), res[1][0] = 0.f,		  res[2][0] = 0.f,		  res[3][0] = -(r+l)/(r-l);
		res[0][1] = 0.f,	   res[1][1] = 2.f/(t-b), res[2][1] = 0.f,		  res[3][1] = -(t+b)/(t-b);
		res[0][2] = 0.f,	   res[1][2] = 0.f,		  res[2][2] = -2.f/(f-n), res[3][2] = -(f+n)/(f-n);
		res[0][3] = 0.f,	   res[1][3] = 0.f,		  res[2][3] = 0.f,		  res[3][3] = 1.f;

		// new_x = 2/(r-l) * (x - (r+l/2))
		//       = 2/(r-l) * x - (r+l)/(r-l);
		return res;
	}

	static mat4 perspective(float fovy, float aspect, float n, float f)
	{
		if (USING_GLM_MATRIX) return glm::perspective(fovy, aspect, n, f);

		const float t = abs(n * glm::tan(fovy * 0.5f)), b = -t;
		const float r = abs(t * aspect), l = -r;

		mat4 res;
		res[0][0] = 2.f*n/(r-l), res[1][0] = 0.f,         res[2][0] = 0.f,  res[3][0] = 0.f;
		res[0][1] = 0.f,         res[1][1] = 2.f*n/(t-b), res[2][1] = 0.f,  res[3][1] = 0.f;
		res[0][2] = 0.f,         res[1][2] = 0.f,         res[2][2] = -(f+n)/(f-n), res[3][2] = 2.f*n*f/(n-f);
		res[0][3] = 0.f,         res[1][3] = 0.f,         res[2][3] = -1.f,         res[3][3] = 0.f;

		return res;
	}
};