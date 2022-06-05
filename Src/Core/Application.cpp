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
		else // else�ｫ������Ⱦ���ݣ�û�д�����Ϣ��Ҫ����ʱ������Ⱦ
		{
			Timer timer = { &m_deltaTime };
			InternalInputEvent.Invoke();
			InputEvent.Invoke();
			if (m_pause == false)
			{
				AppUpdateEvent.Invoke();
				RenderEvent.Invoke();
			}
		}
	}
}
