#include "pch.h"

#include "stb_image.h"
#include "Texture.h"

Texture::Texture(const std::string& path) : m_data(nullptr)
{
	stbi_set_flip_vertically_on_load(true); // ʹ��ͼƬ�����������ʼ��Ϊ���½ǣ���uv�����Ӧ
	byte* raw = stbi_load(path.c_str(), &m_width, &m_height, &m_channelCount, 0);
	
	if (raw == nullptr) { TIPS(L"��������ʧ��"); return; }
	if (m_channelCount != 1 && m_channelCount != 3 && m_channelCount != 4) { TIPS(L"δ�������ɫͨ������"); return; }

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
	stbi_image_free(raw); // �ͷſռ�
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
	// repeat wrap mode��������0, 1����Χ��uv������Ȼ�ظ�����ͼ��
	int x = uv.x * m_width, y = uv.y * m_height;
	if (x >= 0) { x %= m_width; }
	else { x = m_width - 1 - ((-x) % m_width); }
	if (y >= 0) { y %= m_height;  }
	else { y = m_height - 1 - ((-y) % m_height); }
	unsigned int i = x + y * m_width;
	return m_data[i];
}

vec4 Sample2D(Texture& tex, const vec2& uv)
{
	return tex.GetColor(uv);
}
