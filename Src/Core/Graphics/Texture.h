#pragma once

class Texture
{
	friend vec4 sample2D(Texture& tex, const vec2& uv);
public:
	Texture(const std::string& path);
	Texture(const Texture& other);
	Texture(Texture&& other) noexcept;
	~Texture();

private:
	vec4 GetColor(const vec2& uv);

private:
	int m_channelCount;
	int m_width, m_height;

	byte* m_data;
};

vec4 sample2D(Texture& tex, const vec2& uv);