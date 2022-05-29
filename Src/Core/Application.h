#pragma once

class Application : public Singleton<Application>
{
public:
	void Init(LPCWSTR appName);
	
	LPCWSTR Name() const { return m_appName; };
	Event InputEvent;
	Event AppUpdateEvent;

	void Run();

public:
	float GetDeltaTime() const { return m_deltaTime; }

private:
	LPCWSTR m_appName;
	float m_deltaTime;
};
