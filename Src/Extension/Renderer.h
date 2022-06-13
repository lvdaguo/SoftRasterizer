#pragma once
#include "pch.h"

class Renderer : public Singleton<Renderer>
{
public:
	virtual ~Renderer() override { }
	void Init() 
	{ 
		LOG_TRACE("init renderer");
	}
};