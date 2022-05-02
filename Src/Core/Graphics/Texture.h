#pragma once



class Texture
{
	friend vec4 sample2D(Texture& tex, const vec2& uv);
public:
	Texture(const std::string& path);
	~Texture();

private:
	vec4 GetColor(const vec2& uv);

private:
	int m_channelCount;
	int m_width, m_height;

	vec4* m_data;
};

vec4 sample2D(Texture& tex, const vec2& uv);