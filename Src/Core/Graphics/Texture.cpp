#include "pch.h"

#include "stb_image.h"
#include "Texture.h"

Texture::Texture(const std::string& path)
{
	stbi_set_flip_vertically_on_load(true);
	m_data = stbi_load(path.c_str(), &m_width, &m_height, &m_channelCount, 0);
	if (m_data == nullptr) { TIPS(L"加载纹理失败"); }
	else if (m_channelCount != 1 && m_channelCount != 3 && m_channelCount != 4) { TIPS(L"未定义的颜色通道数量"); }
}

Texture::Texture(const Texture& other)
{
	m_channelCount = other.m_channelCount;
	m_width = other.m_width;
	m_height = other.m_height;
	unsigned int len = m_width * m_height;
	m_data = new byte[len];
	for (unsigned int i = 0; i < len; ++i)
	{
		m_data[i] = other.m_data[i];
	}
}

Texture::Texture(Texture&& other) noexcept
{
	m_channelCount = other.m_channelCount;
	m_width = other.m_width;
	m_height = other.m_height;
	m_data = other.m_data;
	other.m_data = nullptr;
}

Texture::~Texture()
{
	if (m_data != nullptr)
	{
		stbi_image_free(m_data); // 释放空间
		m_data = nullptr;
	}
}

vec4 Texture::GetColor(const vec2& uv)
{
	unsigned int x = uv.x * m_width, y = uv.y * m_height;
	unsigned int i = x + y * m_width;
	if (m_channelCount == 1) 
	{ 
		byte* data = m_data;
		byte& res = data[i];
		return vec4(res / 255.0f);
	}
	else if (m_channelCount == 3)
	{
		vec3b* data = (vec3b*)m_data;
		vec3b& res = data[i];
		return { (float)res.r / 255.0f, (float)res.g / 255.0f, (float)res.b / 255.0f , 1.0f};
	} 
	else // (m_channelCount == 4)
	{
		vec4b* data = (vec4b*)m_data;
		vec4b& res = data[i];
		return { (float)res.r / 255.0f, (float)res.g / 255.0f, (float)res.b / 255.0f, (float)res.a / 255.0f };
	}
	return vec4(1.0f);
}

vec4 sample2D(Texture& tex, const vec2& uv)
{
	return tex.GetColor(uv);
}
