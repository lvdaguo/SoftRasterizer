#pragma once

#include "pch.h"
#include "Core/Graphics/Texture.h"

// application to vertex shader
struct a2v
{
	int index;							 // 当前vertex在vertex buffer中的索引
	void* vb;							 // vertex buffer 原始数据指针

	std::unordered_map<int, float> f1;   // 浮点数 varying 列表
	std::unordered_map<int, vec2> f2;    // 二维矢量 varying 列表
	std::unordered_map<int, vec3> f3;    // 三维矢量 varying 列表
	std::unordered_map<int, vec4> f4;    // 四维矢量 varying 列表

	void clear()
	{
		f1.clear();
		f2.clear();
		f3.clear();
		f4.clear();
	}
};

// vertex shader to fragment shader
struct v2f
{
	Ref<Texture>* textures;
	std::unordered_map<int, float> f1;   // 浮点数 varying 列表
	std::unordered_map<int, vec2> f2;    // 二维矢量 varying 列表
	std::unordered_map<int, vec3> f3;    // 三维矢量 varying 列表
	std::unordered_map<int, vec4> f4;    // 四维矢量 varying 列表
};

// 顶点着色器：因为是 C++ 编写，无需传递 attribute，传递顶点序号
// 着色器函数直接在外层根据序号读取相应数据即可，最后需要返回一个坐标 pos
// 各项 varying 设置到 output 里，由渲染器插值后传递给 FS 
typedef std::function<vec4(a2v& appdata)> VertexShader;

// 片段着色器：需要返回 vec4 类型的颜色
// 三角形内每个片段的 input 具体值会根据前面三个顶点的 output 插值得到
typedef std::function<vec4(v2f& input)> FragmentShader;

class ShaderProgram
{
public:
	ShaderProgram(VertexShader vs, FragmentShader fs) : m_vertexShader(vs), m_fragmentShader(fs) { }
	VertexShader GetVertexShader() const { return m_vertexShader; }
	FragmentShader GetFragmentShader() const { return m_fragmentShader; }

private:
	VertexShader m_vertexShader;
	FragmentShader m_fragmentShader;
};