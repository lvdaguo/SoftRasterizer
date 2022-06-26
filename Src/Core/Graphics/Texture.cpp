#include "pch.h"

#include "stb_image.h"
#include "Texture.h"

Texture::Texture(const std::string& path, bool isBilinearSampling) : m_data(nullptr), m_isBilinearSampling(isBilinearSampling)
{
	stbi_set_flip_vertically_on_load(true); // 使得图片输出的像素起始点为左下角，和uv坐标对应
	byte* raw = stbi_load(path.c_str(), &m_width, &m_height, &m_channelCount, 0);
	
	if (raw == nullptr) { LOG_ERROR("加载纹理失败，路径为{}", path); return; }
	if (m_channelCount != 1 && m_channelCount != 3 && m_channelCount != 4) { LOG_ERROR("纹理错误：未定义的颜色通道数量"); return; }

	m_data = new vec4[m_width * m_height];
	for (int i = 0; i < m_width * m_height; ++i)
	{
		int ri = i * m_channelCount;
		if (m_channelCount == 1)
		{
			m_data[i] = { raw[ri] / 255.f, raw[ri] / 255.f, raw[ri] / 255.f, 1.f };
		}
		else if (m_channelCount == 3)
		{
			m_data[i] = { raw[ri] / 255.f, raw[ri + 1] / 255.f, raw[ri + 2] / 255.f, 1.f };
		}
		else // (m_channelCount == 4)
		{
			m_data[i] = { raw[ri] / 255.f, raw[ri + 1] / 255.f, raw[ri + 2] / 255.f, raw[ri + 3] / 255.f };
		}
	}
	stbi_image_free(raw); // 释放空间
}

Texture::~Texture()
{
	if (m_data != nullptr)
	{
		delete[] m_data;
	}
}

vec4 Texture::GetColor(const vec2& uv)
{
	if (m_isBilinearSampling)
	{
		float u = uv.x * m_width;
		float v = uv.y * m_height;
		float u_min = floor(u), u_max = ceil(u);
		float v_min = floor(v), v_max = ceil(v);

		vec4 c00 = GetPointColor({u_min / m_width, v_min / m_height});
		vec4 c01 = GetPointColor({u_min / m_width, v_max / m_height});
		vec4 c10 = GetPointColor({u_max / m_width, v_min / m_height});
		vec4 c11 = GetPointColor({u_max / m_width, v_max / m_height});

		auto lerp = [](const vec4& c0, const vec4& c1, float x) -> vec4
		{
			return (1.f - x) * c0 + x * c1;
		};

		float s = (u - u_min) / (u_max - u_min);
		float t = (v - v_min) / (v_max - v_min);

		vec4 c0 = lerp(c00, c10, s);
		vec4 c1 = lerp(c01, c11, s);
		vec4 color = lerp(c0, c1, t);

		return color;
	}
	
	return GetPointColor(uv);
}

vec4 Texture::GetPointColor(const vec2& uv)
{
	// repeat wrap mode，超出【0, 1】范围的uv坐标依然重复纹理图像
	int x = uv.x * m_width, y = uv.y * m_height;
	if (x >= 0) { x %= m_width; }
	else { x = m_width - 1 - ((-x) % m_width); }
	if (y >= 0) { y %= m_height; }
	else { y = m_height - 1 - ((-y) % m_height); }
	unsigned int i = x + y * m_width;
	return m_data[i];
}

vec4 Sample2D(Texture& tex, const vec2& uv)
{
	return tex.GetColor(uv);
}

void ToggleSampleMode(Texture& tex)
{
	tex.SetBilinearSampling(!tex.IsBilinearSampling());
}
