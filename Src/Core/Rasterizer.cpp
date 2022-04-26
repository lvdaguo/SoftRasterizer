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

	// ������ͼ���õ�λͼ
	void* bmpBuffer = NULL;

	m_hDC = GetDC(window.GetHWND());// ��ʾ
	m_hMem = CreateCompatibleDC(m_hDC);// �滭

	BITMAPINFO bmpInfo;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFO);		// ����ṹ�ĳ����ǹ̶��ģ�Ϊ40���ֽ�(LONGΪ32λ����)
	bmpInfo.bmiHeader.biWidth = m_width;		// ָ��ͼ��Ŀ�ȣ���λ������
	bmpInfo.bmiHeader.biHeight = m_height;	// ָ��ͼ��ĸ߶ȣ���λ������
	bmpInfo.bmiHeader.biPlanes = 1;						// ������1�����ÿ���
	bmpInfo.bmiHeader.biBitCount = 32;					// ָ����ʾ��ɫʱҪ�õ���λ�������õ�ֵΪ1(�ڰ׶�ɫͼ), 4(16ɫͼ), 8(256ɫ), 24(���ɫͼ)(�µ�.bmp��ʽ֧��32λɫ
	bmpInfo.bmiHeader.biCompression = BI_RGB;			// ʵ�ʴ洢��ʽΪBGR

	bmpInfo.bmiHeader.biSizeImage = 0;
	// ָ��ʵ�ʵ�λͼ����ռ�õ��ֽ�������ʵҲ���Դ����µĹ�ʽ�м������ biSizeImage=biWidth�� �� biHeight
	// Ҫע����ǣ�������ʽ�е�biWidth��������4��������(���Բ���biWidth������biWidth������ʾ���ڻ����biWidth�ģ���ӽ�4����������
	// �ٸ����ӣ����biWidth=240����biWidth��=240�����biWidth=241��biWidth��=244
	// ���biCompressionΪBI_RGB����������Ϊ��

	bmpInfo.bmiHeader.biXPelsPerMeter = 0;        // ָ��Ŀ���豸��ˮƽ�ֱ��ʣ���λ��ÿ�׵����ظ���
	bmpInfo.bmiHeader.biYPelsPerMeter = 0;        // ָ��Ŀ���豸�Ĵ�ֱ�ֱ��ʣ���λͬ��
	bmpInfo.bmiHeader.biClrUsed = 0;              // ָ����ͼ��ʵ���õ�����ɫ���������ֵΪ�㣬���õ�����ɫ��Ϊ2biBitCount
	bmpInfo.bmiHeader.biClrImportant = 0;         // ָ����ͼ������Ҫ����ɫ���������ֵΪ�㣬����Ϊ���е���ɫ������Ҫ��

	// �����ﴴ��bmpBuffer�ڴ�
	// CreateDIBSection���������λͼ�ṹ��Ϣ��pbmi�������ڴ�ռ䣬�㲻��Ϊ�������ڴ棬����ڴ�Ҳ����Ҫ���ͷţ�ϵͳ���Լ��ͷŵġ�   
	// Ȼ��λͼ�е�ͼ�����ݶ�������ڴ��ַ����ʾ����
	HBITMAP hBmp = CreateDIBSection(m_hDC, &bmpInfo, DIB_RGB_COLORS, (void**)&bmpBuffer, 0, 0);

	SelectObject(m_hMem, hBmp);//��һ������(λͼ�����ʡ���ˢ��)ѡ��ָ�����豸������

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
	// ������һ֡����ǰ��Ҫ����
	m_colorBuffer->FillColor(m_clearColor);

	// �����ﻭ���豸�ϣ�hMem�൱�ڻ�����
	BitBlt(m_hDC, 0, 0, m_width, m_height, m_hMem, 0, 0, SRCCOPY);
	// �ú�����ָ����Դ�豸���������е����ؽ���λ�飨bit_block��ת�����Դ��͵�Ŀ���豸����
}
