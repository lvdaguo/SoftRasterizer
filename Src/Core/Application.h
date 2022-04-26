#pragma once

class Application : public Singleton<Application>
{
public:
	Application() : m_appName(nullptr) { }

	void Init(LPCWSTR appName);

	void Run();

	LPCWSTR Name() const { return m_appName; };

private:
	LPCWSTR m_appName;
};