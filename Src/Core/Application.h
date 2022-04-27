#pragma once

class Application : public Singleton<Application>
{
public:
	Application() : m_appName(nullptr) { }
	void Init(LPCWSTR appName);
	
	LPCWSTR Name() const { return m_appName; };
	Event AppUpdateEvent;

	void Run();

private:
	LPCWSTR m_appName;
};