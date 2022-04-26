#pragma once

typedef unsigned char byte;

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;


inline float deg2rad(float deg)
{
	return deg / 180.0f * 3.1415926;
}

static unsigned int s_curID = 1;

inline unsigned int GenerateID()
{
	return s_curID++;
}

//ÊÂ¼þ°ó¶¨ºê
#define BIND(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
