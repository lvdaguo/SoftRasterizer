 #include "pch.h"
#include "Core/Application.h"

#include "Window.h"

void Window::Init(HINSTANCE hInstance, unsigned int width, unsigned int height, Application& app)
{
	// 注册窗口类，指定窗口类的名称与窗口回调函数
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);			// 该对象实际占用多大内存
	windowClass.cbClsExtra = 0;							// 是否需要额外空间
	windowClass.cbWndExtra = 0;							// 是否需要额外内存
	windowClass.hbrBackground = NULL;					// 如果有设置哪就是GDI擦除
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);	// 设置一个箭头光标
	windowClass.hIcon = NULL;							// 应用程序在磁盘上显示的图标
	windowClass.hIconSm = NULL;							// 应用程序显示在左上角的图标1
	windowClass.hInstance = hInstance;					// 窗口实例
	windowClass.lpszClassName = app.Name();				// 窗口名字
	windowClass.lpszMenuName = NULL;
	windowClass.style = CS_VREDRAW | CS_HREDRAW;		// 怎么绘制窗口垂直和水平重绘

	windowClass.lpfnWndProc = WindowProc;				// 消息处理函数

	if (!RegisterClassEx(&windowClass))
	{
		MessageBox(NULL, L"注册窗口类失败！", L"Tip", NULL);
		return;
	}

	// 创建窗口，指定注册窗口类，窗口标题，窗口的大小
	RECT Rect = { 0, 0, width, height };
	// 客户矩形是指完全包含一个客户区域的最小矩形；窗口矩形是指完全包含一个窗口的最小矩形，该窗口包含客户区与非客户区。
	AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, false);	// 根据窗口的客户区大小计算窗口的大小
	int newWidth = Rect.right - Rect.left;
	int newHeight = Rect.bottom - Rect.top;
	// 声明窗口句柄
	HWND hWnd = CreateWindowEx
	(
		NULL,					// 窗口额外风格
		app.Name(),				// 窗口名称
		app.Name(),				// 会显示在窗口标题栏上
		(WS_OVERLAPPEDWINDOW  & (~WS_MAXIMIZEBOX)) ^ WS_THICKFRAME,
		// 窗口风格（THICKFRAME运行时不允许改变窗口大小，不允许最大化）
		100, 100,				// 窗口的坐标
		newWidth, newHeight,
		NULL,					// 副窗口句柄
		NULL,					// 菜单句柄
		hInstance,				// 窗口实例
		NULL
	);
	if (!hWnd) 
	{
		MessageBox(NULL, L"创建窗口失败！", L"Tip", NULL);
		return;
	}

	m_width = newWidth;
	m_height = newHeight;
	m_hWnd = hWnd;

	// 显示窗口，更新窗口
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
		// 当窗口被销毁时，终止消息循环
		case WM_DESTROY:
			PostQuitMessage(0); // 终止消息循环，并发出WM_QUIT消息，退出程序
			return 0;
	}

	// 将上面没有处理的消息转发给默认的窗口过程
	return DefWindowProc(hWnd, message, wParam, lParam);
}