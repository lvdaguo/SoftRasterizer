#include "Core/GlobalHeaders.h"
#include "Core/GlobalParams.h"

#include "Core/Window.h"
#include "Core/Application.h"

Window& window = Window::Instance();
Application& app = Application::Instance();

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	app.Init(APP_NAME);
	window.Init(hInstance, WIDTH, HEIGHT, app);

	app.Run();

	return 0;
}