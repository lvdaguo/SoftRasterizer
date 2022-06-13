#pragma once
#include "Sample.h"
#include "Extension/DemoCamera.h"

class ModelSample : public Sample
{
public:
	ModelSample();

	virtual void OnUpdate() override;

private:
	DemoCamera m_cam;
};

