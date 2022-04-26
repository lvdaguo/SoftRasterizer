#include "Core/GlobalHeaders.h"

#include "Core/Buffer/ColorBuffer.h"
#include "Core/Window.h"
#include "Core/Application.h"
#include "Rasterizer.h"

Rasterizer::~Rasterizer()
{
	Application::Instance().AppUpdateEvent -= m_onUpdate;
	delete m_colorBuffer;
}

void Rasterizer::Init()
{
	m_onUpdate = Action(BIND(OnUpdate));
	Application::Instance().AppUpdateEvent += m_onUpdate;

	Window& window = Window::Instance();
	m_width = window.GetWidth();
	m_height = window.GetHeight();

	// 创建绘图所用的位图
	void* bmpBuffer = NULL;

	m_hDC = GetDC(window.GetHWND());// 显示
	m_hMem = CreateCompatibleDC(m_hDC);// 绘画

	BITMAPINFO bmpInfo;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFO);		// 这个结构的长度是固定的，为40个字节(LONG为32位整数)
	bmpInfo.bmiHeader.biWidth = m_width;		// 指定图象的宽度，单位是象素
	bmpInfo.bmiHeader.biHeight = m_height;	// 指定图象的高度，单位是象素
	bmpInfo.bmiHeader.biPlanes = 1;						// 必须是1，不用考虑
	bmpInfo.bmiHeader.biBitCount = 32;					// 指定表示颜色时要用到的位数，常用的值为1(黑白二色图), 4(16色图), 8(256色), 24(真彩色图)(新的.bmp格式支持32位色
	bmpInfo.bmiHeader.biCompression = BI_RGB;			// 实际存储方式为BGR

	bmpInfo.bmiHeader.biSizeImage = 0;
	// 指定实际的位图数据占用的字节数，其实也可以从以下的公式中计算出来 biSizeImage=biWidth’ × biHeight
	// 要注意的是：上述公式中的biWidth’必须是4的整倍数(所以不是biWidth，而是biWidth’，表示大于或等于biWidth的，最接近4的整倍数。
	// 举个例子，如果biWidth=240，则biWidth’=240；如果biWidth=241，biWidth’=244
	// 如果biCompression为BI_RGB，则该项可能为零

	bmpInfo.bmiHeader.biXPelsPerMeter = 0;        // 指定目标设备的水平分辨率，单位是每米的像素个数
	bmpInfo.bmiHeader.biYPelsPerMeter = 0;        // 指定目标设备的垂直分辨率，单位同上
	bmpInfo.bmiHeader.biClrUsed = 0;              // 指定本图像实际用到的颜色数，如果该值为零，则用到的颜色数为2biBitCount
	bmpInfo.bmiHeader.biClrImportant = 0;         // 指定本图象中重要的颜色数，如果该值为零，则认为所有的颜色都是重要的

	// 在这里创建bmpBuffer内存
	// CreateDIBSection函数会根据位图结构信息（pbmi）分配内存空间，你不用为它分配内存，这块内存也不需要你释放，系统会自己释放的。   
	// 然后将位图中的图像数据读入这个内存地址，显示即可
	HBITMAP hBmp = CreateDIBSection(m_hDC, &bmpInfo, DIB_RGB_COLORS, (void**)&bmpBuffer, 0, 0);

	SelectObject(m_hMem, hBmp);//把一个对象(位图、画笔、画刷等)选入指定的设备描述表

	m_clearColor = Color::Black();
	m_colorBuffer = new ColorBuffer(bmpBuffer, m_width, m_height);
}

void Rasterizer::DrawTriangle()
{
}

void Rasterizer::DrawBuffer()
{
}

void Rasterizer::SetClearColor(Color color)
{
	m_clearColor = color;
}

void Rasterizer::OnUpdate()
{
	// 进行下一帧绘制前需要清屏
	m_colorBuffer->FillColor(m_clearColor);

	// 在这里画到设备上，hMem相当于缓冲区
	BitBlt(m_hDC, 0, 0, m_width, m_height, m_hMem, 0, 0, SRCCOPY);
	// 该函数对指定的源设备环境区域中的像素进行位块（bit_block）转换，以传送到目标设备环境
}
