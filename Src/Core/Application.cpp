#include "Core/GlobalHeaders.h"

#include "Application.h"

void Application::Init(LPCWSTR appName)
{
	m_appName = appName;
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
		// else�ｫ������Ⱦ���ݣ�û�д�����Ϣ��Ҫ����ʱ������Ⱦ
		else
		{

		}
	}
}
