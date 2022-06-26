#pragma once

class Texture
{
	friend vec4 Sample2D(Texture& tex, const vec2& uv);
public:
	Texture(const std::string& path, bool isBilinear = true);
	~Texture();

public:
	bool IsBilinearSampling() const { return m_isBilinearSampling; }
	void SetBilinearSampling(bool enable) { m_isBilinearSampling = enable; }

private:
	vec4 GetColor(const vec2& uv);
	vec4 GetPointColor(const vec2& uv);

private:
	int m_channelCount;
	int m_width, m_height;
	bool m_isBilinearSampling;

	vec4* m_data;
};

vec4 Sample2D(Texture& tex, const vec2& uv);
void ToggleSampleMode(Texture& tex);