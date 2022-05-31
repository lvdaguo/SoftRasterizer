#pragma once

class Application : public Singleton<Application>
{
public:
	void Init(LPCWSTR appName);

public:
	void Run();
	void Pause() { m_pause = true; }
	void Unpause() { m_pause = false; }

public:
	LPCWSTR Name() const { return m_appName; };
	bool IsPaused() const { return m_pause; }
	float GetDeltaTime() const { return m_deltaTime; }

public:
	Event InputEvent;
	Event AppUpdateEvent;
	Event RenderEvent;

private:
	LPCWSTR m_appName;
	float m_deltaTime;
	bool m_pause;
};
