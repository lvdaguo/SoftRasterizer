#include "pch.h"

#include "Application.h"

void Application::Init(LPCWSTR appName)
{
	m_pause = false;
	m_deltaTime = 1.0f / 30.0f;
	m_appName = appName;
	LOG_TRACE("init application");
}

void Application::Run()
{
	// ��ʼ��Ϣѭ��
	// ������Ϣ�ṹ��
	MSG msg = { 0 };
	// ���PeekMessage����������0��˵��û�н��ܵ�WM_QUIT
	while (msg.message != WM_QUIT)
	{
		// ����д�����Ϣ�ͽ��д���
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);	// ���̰���ת�������������Ϣת��Ϊ�ַ���Ϣ
			DispatchMessage(&msg);	// ����Ϣ���ɸ���Ӧ�Ĵ��ڹ���
		}
		// else // else�ｫ������Ⱦ���ݣ�û�д�����Ϣ��Ҫ����ʱ������Ⱦ
		{
			if (m_pause == false)
			{
				LOG_WARN("{0:.2f}ms, FPS {1:d}", m_deltaTime * 1000.0f, static_cast<int>(1.0f / m_deltaTime));
				Timer timer = { &m_deltaTime };
				InputEvent.Invoke();
				AppUpdateEvent.Invoke();
				RenderEvent.Invoke();
			}
			else
			{
				InputEvent.Invoke();
			}
		}
	}
}
