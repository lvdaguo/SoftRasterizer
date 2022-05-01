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
	// 创建绘图所用的位图
	void* bmpBuffer = NULL;

	m_hDC = GetDC(hWnd);// 显示
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

	return bmpBuffer;
}

void Rasterizer::Draw()
{
	if (m_vertexBuffer == NULL)   { TIPS(L"未绑定VertexBuffer");    return; }
	if (m_indexBuffer == NULL)    { TIPS(L"未绑定IndexBuffer");     return; }
	if (m_vertexShader == NULL)   { TIPS(L"未绑定VertexShader");    return; }
	if (m_fragmentShader == NULL) { TIPS(L"未绑定FragmentShader");  return; }

	VertexBuffer& vb = *m_vertexBuffer;
	IndexBuffer& ib = *m_indexBuffer;

	std::unordered_set<unsigned int> discarded;

	for (unsigned int i : ib)
	{
		Vertex& v = vb[i];
		v.appdata.index = i; // 设置index
	}

	for (Vertex& v : vb) { v.appdata.clear(); } // 清空上下文 varying 列表

	// vertex shader 运行顶点着色程序，返回顶点坐标
	for (Vertex& v : vb) { v.pos = m_vertexShader(v.appdata); }

	// clipping 简单裁剪，任何一个顶点超过 CVV 就剔除
	for (Vertex& v : vb)
	{
		if (v.pos.w == 0.0f) { discarded.insert(v.appdata.index); continue; }
		if (v.pos.z < 0.0f || v.pos.z > v.pos.w) { discarded.insert(v.appdata.index); continue; }
		if (v.pos.x < -v.pos.w || v.pos.x > v.pos.w) { discarded.insert(v.appdata.index); continue; }
		if (v.pos.y < -v.pos.w || v.pos.y > v.pos.w) { discarded.insert(v.appdata.index); continue; }
	}

	// clip to ndc（透视除法）
	for (Vertex& v : vb)
	{
		v.rhw = 1.0f / v.pos.w;	// w 的倒数：Reciprocal of the Homogeneous W 
		v.pos /= v.pos.w;	    // 齐次坐标空间 /w 归一化到单位体积cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1
	}

	// ndc to screen (计算屏幕坐标)
	for (Vertex& v : vb)
	{
		float x = (v.pos.x + 1.0f) * m_width * 0.5f;
		float y = (1.0f - v.pos.y) * m_height * 0.5f;
		v.spos = { x, y, 0.f };
	}

	// triangle assembly （装配三角形）
	std::vector<std::pair<Triangle, Rect>> que;
	que.reserve(ib.GetCount() / 3);
	for (unsigned int j = 0; j < ib.GetCount(); j += 3)
	{
		bool discard = false;
		for (unsigned int k = 0; k < 3; ++k) { if (discarded.count(j + k)) { discard = true; break; } }
		if (discard) continue; // 该三角形已经放弃绘制

		Vertex* p2v[3] = { };
		vec2i min = { m_width - 1, m_height - 1 }, max = { 0, 0 };
		for (unsigned int k = 0; k < 3; ++k)
		{
			unsigned int i = ib[j + k];
			Vertex& v = vb[i];
			p2v[k] = &vb[i];

			min.x = glm::min(min.x, (int)(v.spos.x + 0.5f)); // 整数屏幕坐标：加 0.5 的偏移取屏幕像素方格中心对齐
			min.y = glm::min(min.y, (int)(v.spos.y + 0.5f));
			max.x = glm::max(max.x, (int)(v.spos.x + 0.5f));
			max.y = glm::max(max.y, (int)(v.spos.y + 0.5f));
		}
		que.push_back({ { p2v[0], p2v[1], p2v[2] }, { min, max } });
	}

	// scan pixel block 迭代三角形外接矩形的所有点
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

	// per fragment 逐片段
	for (auto& [tri, px] : frags)
	{
		// calculate barycentric coordinates 计算重心坐标
		vec3 p2a = tri.a->spos - px; // 三个端点到当前点的矢量
		vec3 p2b = tri.b->spos - px;
		vec3 p2c = tri.c->spos - px;

		float sa = -glm::cross(p2b, p2c).z;    // 子三角形 p-b-c 面积（面朝方向为z轴正方向，叉积后的法向量的z为负）
		float sb = -glm::cross(p2c, p2a).z;    // 子三角形 p-c-a 面积
		float sc = -glm::cross(p2a, p2b).z;    // 子三角形 p-a-b 面积
		float s = sa + sb + sc;				   // 大三角形 a-b-c 面积

		if (sa < 0.f || sb < 0.f || sc < 0.f) {	continue; } // 重心（像素）不在三角形中
		if (s == 0.f)						  { continue; } // 三角形面积为0

		float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // 得到重心坐标
		float rhw = alpha * tri.a->rhw + beta * tri.b->rhw + gamma * tri.c->rhw; // 重心插值后的w倒数
		float w = 1.f / rhw;

		// barycentric interpolation 重心坐标插值
		a2v& data = tri.a->appdata;
		Vertex& a = *tri.a, &b = *tri.b, &c = *tri.c;
		a2v& ad = a.appdata, &bd = b.appdata, &cd = c.appdata;
		v2f in;
		in.textures = m_textureSlots;
		for (auto& [k, v] : data.f1) { in.f1[k] = (ad.f1[k] * a.rhw * alpha + bd.f1[k] * b.rhw * beta + cd.f1[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f2) { in.f2[k] = (ad.f2[k] * a.rhw * alpha + bd.f2[k] * b.rhw * beta + cd.f2[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f3) { in.f3[k] = (ad.f3[k] * a.rhw * alpha + bd.f3[k] * b.rhw * beta + cd.f3[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f4) { in.f4[k] = (ad.f4[k] * a.rhw * alpha + bd.f4[k] * b.rhw * beta + cd.f4[k] * c.rhw * gamma) * w; }
	
		// fragment shader 片段着色器
		vec4 color = m_fragmentShader(in);

		// depth test 深度测试
		float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * rhw;
		if (z >= m_depthBuffer->data(px.x, px.y)) { continue; }
		m_depthBuffer->data(px.x, px.y) = z;

		// blending 混合
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
	// 在这里画到设备上，hMem相当于缓冲区
	BitBlt(m_hDC, 0, 0, m_width, m_height, m_hMem, 0, 0, SRCCOPY);
	// 该函数对指定的源设备环境区域中的像素进行位块（bit_block）转换，以传送到目标设备环境
}