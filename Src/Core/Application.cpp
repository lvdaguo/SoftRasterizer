#include "pch.h"

#include "Application.h"

void Application::Init(LPCWSTR appName)
{
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
			auto bef = std::chrono::high_resolution_clock::now();
			InputEvent.Invoke();
			AppUpdateEvent.Invoke();
			auto aft = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(aft - bef).count();
			m_deltaTime = static_cast<float>(duration * 0.000000001);
		}
	}
}
