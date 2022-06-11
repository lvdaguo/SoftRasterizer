 #include "pch.h"
#include "Core/Application.h"

#include "Window.h"

void Window::Init(HINSTANCE hInstance, unsigned int width, unsigned int height, Application& app)
{
	// ע�ᴰ���ָ࣬��������������봰�ڻص�����
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);			// �ö���ʵ��ռ�ö���ڴ�
	windowClass.cbClsExtra = 0;							// �Ƿ���Ҫ����ռ�
	windowClass.cbWndExtra = 0;							// �Ƿ���Ҫ�����ڴ�
	windowClass.hbrBackground = NULL;					// ����������ľ���GDI����
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);	// ����һ����ͷ���
	windowClass.hIcon = NULL;							// Ӧ�ó����ڴ�������ʾ��ͼ��
	windowClass.hIconSm = NULL;							// Ӧ�ó�����ʾ�����Ͻǵ�ͼ��1
	windowClass.hInstance = hInstance;					// ����ʵ��
	windowClass.lpszClassName = app.Name();				// ��������
	windowClass.lpszMenuName = NULL;
	windowClass.style = CS_VREDRAW | CS_HREDRAW;		// ��ô���ƴ��ڴ�ֱ��ˮƽ�ػ�

	windowClass.lpfnWndProc = WindowProc;				// ��Ϣ������

	if (!RegisterClassEx(&windowClass))
	{
		MessageBox(NULL, L"ע�ᴰ����ʧ�ܣ�", L"Tip", NULL);
		return;
	}

	// �������ڣ�ָ��ע�ᴰ���࣬���ڱ��⣬���ڵĴ�С
	RECT Rect = { 0, 0, width, height };
	// �ͻ�������ָ��ȫ����һ���ͻ��������С���Σ����ھ�����ָ��ȫ����һ�����ڵ���С���Σ��ô��ڰ����ͻ�����ǿͻ�����
	AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, false);	// ���ݴ��ڵĿͻ�����С���㴰�ڵĴ�С
	int newWidth = Rect.right - Rect.left;
	int newHeight = Rect.bottom - Rect.top;
	// �������ھ��
	HWND hWnd = CreateWindowEx
	(
		NULL,					// ���ڶ�����
		app.Name(),				// ��������
		app.Name(),				// ����ʾ�ڴ��ڱ�������
		(WS_OVERLAPPEDWINDOW  & (~WS_MAXIMIZEBOX)) ^ WS_THICKFRAME,
		// ���ڷ��THICKFRAME����ʱ������ı䴰�ڴ�С����������󻯣�
		100, 100,				// ���ڵ�����
		newWidth, newHeight,
		NULL,					// �����ھ��
		NULL,					// �˵����
		hInstance,				// ����ʵ��
		NULL
	);
	if (!hWnd) 
	{
		MessageBox(NULL, L"��������ʧ�ܣ�", L"Tip", NULL);
		return;
	}

	m_width = newWidth;
	m_height = newHeight;
	m_hWnd = hWnd;

	// ��ʾ���ڣ����´���
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	LOG_TRACE("init window");
	LOG_TRACE("window size: {}, {}", m_width, m_height);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window::Instance().MsgReceivedEvent.Invoke(hWnd, message, wParam, lParam);

	switch (message)
	{
		// �����ڱ�����ʱ����ֹ��Ϣѭ��
		case WM_DESTROY:
			PostQuitMessage(0); // ��ֹ��Ϣѭ����������WM_QUIT��Ϣ���˳�����
			return 0;
	}

	// ������û�д������Ϣת����Ĭ�ϵĴ��ڹ���
	return DefWindowProc(hWnd, message, wParam, lParam);
}