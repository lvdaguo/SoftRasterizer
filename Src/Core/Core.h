#pragma once

typedef unsigned char byte;

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::ivec2 vec2i;
typedef glm::ivec3 vec3i;
typedef glm::ivec4 vec4i;

typedef glm::u8vec2 vec2b;
typedef glm::u8vec3 vec3b;
typedef glm::u8vec4 vec4b;

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename ... Args>
inline Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

static unsigned int s_curID = 1;

inline unsigned int GenerateID()
{
	return s_curID++;
}

//事件绑定宏

#define TIPS(msg) MessageBox(NULL, msg, L"提示", NULL)