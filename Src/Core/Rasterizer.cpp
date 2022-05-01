#include "pch.h"

#include "Core/Graphics/Shader.h"
#include "Core/Graphics/Texture.h"
#include "Core/Window.h"
#include "Core/Application.h"
#include "Rasterizer.h"

Rasterizer::~Rasterizer() { }

void Rasterizer::Init(VertexShader vs, FragmentShader fs)
{
	m_drawWireFrame = false;

	m_vertexShader = vs;
	m_fragmentShader = fs;

	Window& window = Window::Instance();
	m_width = window.GetWidth();
	m_height = window.GetHeight();

	void* bmpBuffer = CreateBitMap(window.GetHWND());

	m_clearColor = vec3(0.0f);
	m_wireFrameColor = vec3(1.0f);

	m_colorBuffer = CreateRef<ColorBuffer>(bmpBuffer, m_width, m_height);

	m_depthBuffer = CreateRef<DepthBuffer>(m_width, m_height);
}

void* Rasterizer::CreateBitMap(HWND hWnd)
{
	// ������ͼ���õ�λͼ
	void* bmpBuffer = NULL;

	m_hDC = GetDC(hWnd);// ��ʾ
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

	return bmpBuffer;
}

void Rasterizer::Draw()
{
	if (m_vertexBuffer == NULL)   { TIPS(L"δ��VertexBuffer");    return; }
	if (m_indexBuffer == NULL)    { TIPS(L"δ��IndexBuffer");     return; }
	if (m_vertexShader == NULL)   { TIPS(L"δ��VertexShader");    return; }
	if (m_fragmentShader == NULL) { TIPS(L"δ��FragmentShader");  return; }

	VertexBuffer& vb = *m_vertexBuffer;
	IndexBuffer& ib = *m_indexBuffer;

	std::unordered_set<unsigned int> discarded;

	for (unsigned int i : ib)
	{
		Vertex& v = vb[i];
		v.appdata.index = i; // ����index
	}

	for (Vertex& v : vb) { v.appdata.clear(); } // ��������� varying �б�

	// vertex shader ���ж�����ɫ���򣬷��ض�������
	for (Vertex& v : vb) { v.pos = m_vertexShader(v.appdata); }

	// clipping �򵥲ü����κ�һ�����㳬�� CVV ���޳�
	for (Vertex& v : vb)
	{
		if (v.pos.w == 0.0f) { discarded.insert(v.appdata.index); continue; }
		if (v.pos.z < 0.0f || v.pos.z > v.pos.w) { discarded.insert(v.appdata.index); continue; }
		if (v.pos.x < -v.pos.w || v.pos.x > v.pos.w) { discarded.insert(v.appdata.index); continue; }
		if (v.pos.y < -v.pos.w || v.pos.y > v.pos.w) { discarded.insert(v.appdata.index); continue; }
	}

	// clip to ndc��͸�ӳ�����
	for (Vertex& v : vb)
	{
		v.rhw = 1.0f / v.pos.w;	// w �ĵ�����Reciprocal of the Homogeneous W 
		v.pos /= v.pos.w;	    // �������ռ� /w ��һ������λ���cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1
	}

	// ndc to screen (������Ļ����)
	for (Vertex& v : vb)
	{
		float x = (v.pos.x + 1.0f) * m_width * 0.5f;
		float y = (1.0f - v.pos.y) * m_height * 0.5f;
		v.spos = { x, y, 0.f };
	}

	// triangle assembly ��װ�������Σ�
	std::vector<std::pair<Triangle, Rect>> que;
	que.reserve(ib.GetCount() / 3);
	for (unsigned int j = 0; j < ib.GetCount(); j += 3)
	{
		bool discard = false;
		for (unsigned int k = 0; k < 3; ++k) { if (discarded.count(j + k)) { discard = true; break; } }
		if (discard) continue; // ���������Ѿ���������

		Vertex* p2v[3] = { };
		vec2i min = { m_width - 1, m_height - 1 }, max = { 0, 0 };
		for (unsigned int k = 0; k < 3; ++k)
		{
			unsigned int i = ib[j + k];
			Vertex& v = vb[i];
			p2v[k] = &vb[i];

			min.x = glm::min(min.x, (int)(v.spos.x + 0.5f)); // ������Ļ���꣺�� 0.5 ��ƫ��ȡ��Ļ���ط������Ķ���
			min.y = glm::min(min.y, (int)(v.spos.y + 0.5f));
			max.x = glm::max(max.x, (int)(v.spos.x + 0.5f));
			max.y = glm::max(max.y, (int)(v.spos.y + 0.5f));
		}
		que.push_back({ { p2v[0], p2v[1], p2v[2] }, { min, max } });
	}

	// scan pixel block ������������Ӿ��ε����е�
	std::vector<std::pair<Triangle, vec3>> frags;
	unsigned int space = 0;
	for (auto& [tri, rect] : que) { space += (rect.max.x - rect.min.x) * (rect.max.y - rect.max.y); }
	frags.reserve(space);
	for (auto& [tri, rect] : que)
	{
		for (int y = rect.min.y; y <= rect.max.y; y++)
		{
			for (int x = rect.min.x; x <= rect.max.x; x++)
			{
				vec3 px = { (float)x + 0.5f, (float)y + 0.5f, 0.f };
				frags.push_back({ tri, px });
			}
		}
	}

	// per fragment ��Ƭ��
	for (auto& [tri, px] : frags)
	{
		// calculate barycentric coordinates ������������
		vec3 p2a = tri.a->spos - px; // �����˵㵽��ǰ���ʸ��
		vec3 p2b = tri.b->spos - px;
		vec3 p2c = tri.c->spos - px;

		float sa = -glm::cross(p2b, p2c).z;    // �������� p-b-c ������泯����Ϊz�������򣬲����ķ�������zΪ����
		float sb = -glm::cross(p2c, p2a).z;    // �������� p-c-a ���
		float sc = -glm::cross(p2a, p2b).z;    // �������� p-a-b ���
		float s = sa + sb + sc;				   // �������� a-b-c ���

		if (sa < 0.f || sb < 0.f || sc < 0.f) {	continue; } // ���ģ����أ�������������
		if (s == 0.f)						  { continue; } // ���������Ϊ0

		float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // �õ���������
		float rhw = alpha * tri.a->rhw + beta * tri.b->rhw + gamma * tri.c->rhw; // ���Ĳ�ֵ���w����
		float w = 1.f / rhw;

		// barycentric interpolation ���������ֵ
		a2v& data = tri.a->appdata;
		Vertex& a = *tri.a, &b = *tri.b, &c = *tri.c;
		a2v& ad = a.appdata, &bd = b.appdata, &cd = c.appdata;
		v2f in;
		in.textures = m_textureSlots;
		for (auto& [k, v] : data.f1) { in.f1[k] = (ad.f1[k] * a.rhw * alpha + bd.f1[k] * b.rhw * beta + cd.f1[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f2) { in.f2[k] = (ad.f2[k] * a.rhw * alpha + bd.f2[k] * b.rhw * beta + cd.f2[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f3) { in.f3[k] = (ad.f3[k] * a.rhw * alpha + bd.f3[k] * b.rhw * beta + cd.f3[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f4) { in.f4[k] = (ad.f4[k] * a.rhw * alpha + bd.f4[k] * b.rhw * beta + cd.f4[k] * c.rhw * gamma) * w; }
	
		// fragment shader Ƭ����ɫ��
		vec4 color = m_fragmentShader(in);

		// depth test ��Ȳ���
		float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * rhw;
		if (z >= m_depthBuffer->data(px.x, px.y)) { continue; }
		m_depthBuffer->data(px.x, px.y) = z;

		// blending ���
		float srcAlpha = color.a;
		vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
		vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
		m_colorBuffer->SetColor(px.x, px.y, res);
	}
}

void Rasterizer::DrawLine(vec2 p1, vec2 p2)
{

}

void Rasterizer::Clear()
{
	m_colorBuffer->FillColor(m_clearColor);
	m_depthBuffer->Clear();
}

void Rasterizer::SwapBuffer()
{
	// �����ﻭ���豸�ϣ�hMem�൱�ڻ�����
	BitBlt(m_hDC, 0, 0, m_width, m_height, m_hMem, 0, 0, SRCCOPY);
	// �ú�����ָ����Դ�豸���������е����ؽ���λ�飨bit_block��ת�����Դ��͵�Ŀ���豸����
}