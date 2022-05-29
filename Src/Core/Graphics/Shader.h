#pragma once

#include "pch.h"
#include "Core/Graphics/Texture.h"

// application to vertex shader
struct a2v
{
	int index;
	void* vb;

	std::unordered_map<int, float> f1;   // ������ varying �б�
	std::unordered_map<int, vec2> f2;    // ��άʸ�� varying �б�
	std::unordered_map<int, vec3> f3;    // ��άʸ�� varying �б�
	std::unordered_map<int, vec4> f4;    // ��άʸ�� varying �б�

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
	std::unordered_map<int, float> f1;   // ������ varying �б�
	std::unordered_map<int, vec2> f2;    // ��άʸ�� varying �б�
	std::unordered_map<int, vec3> f3;    // ��άʸ�� varying �б�
	std::unordered_map<int, vec4> f4;    // ��άʸ�� varying �б�
};

// ������ɫ������Ϊ�� C++ ��д�����贫�� attribute�����ݶ������
// ��ɫ������ֱ������������Ŷ�ȡ��Ӧ���ݼ��ɣ������Ҫ����һ������ pos
// ���� varying ���õ� output �����Ⱦ����ֵ�󴫵ݸ� PS 
typedef std::function<vec4(a2v& appdata)> VertexShader;

// Ƭ����ɫ������Ҫ���� Vec4f ���͵���ɫ
// ��������ÿ����� input ����ֵ�����ǰ����������� output ��ֵ�õ�
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