#include "Core/GlobalHeaders.h"

#include "Application.h"

void Application::Init(LPCWSTR appName)
{
	m_appName = appName;
}

void Application::Run()
{
	// 开始消息循环
	// 定义消息结构体
	MSG msg = { 0 };
	// 如果PeekMessage函数不等于0，说明没有接受到WM_QUIT
	while (msg.message != WM_QUIT)
	{
		// 如果有窗口消息就进行处理
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);	// 键盘按键转换，将虚拟键消息转换为字符消息
			DispatchMessage(&msg);	// 把消息分派给相应的窗口过程
		}
		// else里将放置渲染内容，没有窗口信息需要处理时进行渲染
		else
		{

		}
	}
}
