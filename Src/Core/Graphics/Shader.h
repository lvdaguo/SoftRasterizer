#pragma once

#include "pch.h"

// application to vertex shader
struct a2v
{
	int index;

	std::unordered_map<int, float> f1;  // ������ varying �б�
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
	std::unordered_map<int, float> f1;  // ������ varying �б�
	std::unordered_map<int, vec2> f2;    // ��άʸ�� varying �б�
	std::unordered_map<int, vec3> f3;    // ��άʸ�� varying �б�
	std::unordered_map<int, vec4> f4;    // ��άʸ�� varying �б�
};

// ������ɫ������Ϊ�� C++ ��д�����贫�� attribute���� �Ķ������
// ��ɫ������ֱ������������Ŷ�ȡ��Ӧ���ݼ��ɣ������Ҫ����һ������ pos
// ���� varying ���õ� output �����Ⱦ����ֵ�󴫵ݸ� PS 
typedef std::function<vec4(a2v& appdata)> VertexShader;

// Ƭ����ɫ�������� ShaderContext����Ҫ���� Vec4f ���͵���ɫ
// ��������ÿ����� input ����ֵ�����ǰ����������� output ��ֵ�õ�
typedef std::function<vec4(v2f& input)> FragmentShader;