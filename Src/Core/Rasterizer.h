#pragma once

// 着色器上下文，由 VS 设置，再由渲染器按像素逐点插值后，供 PS 读取
struct v2f 
{
	std::unordered_map<int, float> varying_float;  // 浮点数 varying 列表
	std::unordered_map<int, vec2> varying_vec2;    // 二维矢量 varying 列表
	std::unordered_map<int, vec3> varying_vec3;    // 三维矢量 varying 列表
	std::unordered_map<int, vec4> varying_vec4;    // 四维矢量 varying 列表

	std::unordered_map<int, float>& f = varying_float;  
	std::unordered_map<int, vec2>& vec2 = varying_vec2;    
	std::unordered_map<int, vec3>& vec3 = varying_vec3;    
	std::unordered_map<int, vec4>& vec4 = varying_vec4;    
};

// 顶点着色器：因为是 C++ 编写，无需传递 attribute，传个 0-2 的顶点序号
// 着色器函数直接在外层根据序号读取相应数据即可，最后需要返回一个坐标 pos
// 各项 varying 设置到 output 里，由渲染器插值后传递给 PS 
typedef std::function<vec4(int index, v2f& output)> VertexShader;


// 片段着色器：输入 ShaderContext，需要返回 Vec4f 类型的颜色
// 三角形内每个点的 input 具体值会根据前面三个顶点的 output 插值得到
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