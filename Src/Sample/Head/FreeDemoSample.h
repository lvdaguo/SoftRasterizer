#pragma once
#include "Sample.h"

#include "Extension/DemoCamera.h"

class FreeDemoSample : public Sample
{
public:
	FreeDemoSample();

	virtual void OnUpdate() override;

private:
	DemoCamera m_cam;
};

