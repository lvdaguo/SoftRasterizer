#pragma once

typedef unsigned char byte;



static unsigned int s_curID = 1;

inline unsigned int GenerateID()
{
	return s_curID++;
}

//�¼��󶨺�
#define BIND(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
