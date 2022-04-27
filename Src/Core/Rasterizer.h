#pragma once

// ��ɫ�������ģ��� VS ���ã�������Ⱦ������������ֵ�󣬹� PS ��ȡ
struct v2f 
{
	std::unordered_map<int, float> varying_float;  // ������ varying �б�
	std::unordered_map<int, vec2> varying_vec2;    // ��άʸ�� varying �б�
	std::unordered_map<int, vec3> varying_vec3;    // ��άʸ�� varying �б�
	std::unordered_map<int, vec4> varying_vec4;    // ��άʸ�� varying �б�

	std::unordered_map<int, float>& f = varying_float;  
	std::unordered_map<int, vec2>& vec2 = varying_vec2;    
	std::unordered_map<int, vec3>& vec3 = varying_vec3;    
	std::unordered_map<int, vec4>& vec4 = varying_vec4;    
};

// ������ɫ������Ϊ�� C++ ��д�����贫�� attribute������ 0-2 �Ķ������
// ��ɫ������ֱ������������Ŷ�ȡ��Ӧ���ݼ��ɣ������Ҫ����һ������ pos
// ���� varying ���õ� output �����Ⱦ����ֵ�󴫵ݸ� PS 
typedef std::function<vec4(int index, v2f& output)> VertexShader;


// Ƭ����ɫ�������� ShaderContext����Ҫ���� Vec4f ���͵���ɫ
// ��������ÿ����� input ����ֵ�����ǰ����������� output ��ֵ�õ�
typedef std::function<vec4(v2f& input)> FragmentShader;


class Rasterizer : public Singleton<Rasterizer>
{
public:
	Rasterizer() { }
	virtual ~Rasterizer() override;

	void Init(VertexShader vs = NULL, FragmentShader fs = NULL);

public:
	void DrawTriangle();
	void DrawIndexed();
	void DrawLine(vec2 p1, vec2 p2);

	void Clear();
	void SwapBuffer();

public:
	void SetClearColor(struct Color color);

	void SetVertexShader(VertexShader vs) { m_vertexShader = vs; }
	void SetFragmentShader(FragmentShader fs) { m_fragmentShader = fs; }

private:
	void OnUpdate();

private:
	Action m_onUpdate;

	VertexShader m_vertexShader;
	FragmentShader m_fragmentShader;

	struct Color m_clearColor;
	class ColorBuffer* m_colorBuffer;

	HDC m_hDC, m_hMem;
	unsigned int m_width, m_height;
};