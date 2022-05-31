#include "pch.h"

#include "Core/Graphics/Shader.h"
#include "Core/Graphics/Texture.h"
#include "Core/Window.h"
#include "Core/Application.h"
#include "Rasterizer.h"

//#define RST_LOG

#ifdef RST_LOG
#define RST_INFO(...) LOG_INFO(__VA_ARGS__)
#define RST_TRACE(...) LOG_TRACE(__VA_ARGS__)
#define RST_ERROR(...) LOG_ERROR(__VA_ARGS__)
#else
#define RST_INFO(...) 
#define RST_TRACE(...) 
#define RST_ERROR(...) 
#endif

#define window Window::Instance()
#define app Application::Instance()

Rasterizer::~Rasterizer() { m_threadPool.Shut(); /*app.RenderEvent -= m_onRender;*/ }

void Rasterizer::Init(unsigned int workThreadCount, VertexShader vs, FragmentShader fs)
{
	m_workThreadCount = workThreadCount;
	if (m_workThreadCount > 1) m_threadPool.Init(workThreadCount);
	
	//m_onRender = { std::bind(&Rasterizer::SwapBuffer, this) };
	//app.RenderEvent += m_onRender;

	m_drawWireFrame = false;

	m_vertexShader = vs;
	m_fragmentShader = fs;

	//Window& window = Window::Instance();
	m_width = window.GetWidth();
	m_height = window.GetHeight();

	void* bmpBuffer = CreateBitMap(window.GetHWND());

	m_clearColor = vec3(0.0f);
	m_wireFrameColor = vec3(1.0f);

	m_colorBuffer = CreateRef<ColorBuffer>(bmpBuffer, m_width, m_height);

	m_depthBuffer = CreateRef<DepthBuffer>(m_width, m_height);

	LOG_TRACE("init rasterizer");
	LOG_TRACE("thread count: {}", m_workThreadCount);
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
	if (m_vertexBuffer == NULL) { TIPS(L"未绑定VertexBuffer");    return; }
	if (m_indexBuffer == NULL) { TIPS(L"未绑定IndexBuffer");     return; }
	if (m_vertexShader == NULL) { TIPS(L"未绑定VertexShader");    return; }
	if (m_fragmentShader == NULL) { TIPS(L"未绑定FragmentShader");  return; }
	
	RST_ERROR("------------------------------------------------");
	float drawCallCost;
	{
		Timer timer = { &drawCallCost };
		if (m_workThreadCount == 1)
		{
			RST_ERROR("[Draw Call] [Single Thread]");
			SingleThreadDraw();
		}
		else
		{
			RST_ERROR("[Draw Call] [Multi Thread] {}", m_workThreadCount);
			MultiThreadDraw();
		}
	}
	RST_ERROR("[Draw Finish] cost: {:.2f}", drawCallCost * 1000.f);
	RST_ERROR("");
}

void Rasterizer::SingleThreadDraw()
{
	VertexBuffer& vb = *m_vertexBuffer;
	IndexBuffer& ib = *m_indexBuffer;

	// 设置index
	for (unsigned int i : ib)
	{
		Vertex& v = vb[i];
		v.appdata.index = i; 
	}

	// vertex shader 运行顶点着色程序，返回顶点坐标
	for (Vertex& v : vb) 
	{ 
		v.discard = false; // 重置丢弃标记
		v.appdata.clear(); // 清空上下文 varying 列表
		v.pos = m_vertexShader(v.appdata);
	}

	for (Vertex& v : vb)
	{
		// clipping 简单裁剪，任何一个顶点超过 CVV 就剔除
		if (v.pos.w == 0.0f) { v.discard = true; continue; }
		if (v.pos.z < -v.pos.w || v.pos.z > v.pos.w) { v.discard = true; continue; }
		if (v.pos.x < -v.pos.w || v.pos.x > v.pos.w) { v.discard = true; continue; }
		if (v.pos.y < -v.pos.w || v.pos.y > v.pos.w) { v.discard = true; continue; }

		// clip to ndc 透视除法
		v.rhw = 1.0f / v.pos.w;	// w 的倒数：Reciprocal of the Homogeneous W 
		v.pos /= v.pos.w;	    // 齐次坐标空间 /w 归一化到单位体积cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1
	
		// ndc to screen 计算屏幕坐标
		float x = (v.pos.x + 1.0f) * m_width * 0.5f;
		float y = (1.0f - v.pos.y) * m_height * 0.5f;
		v.spos = { x, y, 0.f };
	}

	// triangle assembly 装配三角形
	std::vector<std::pair<Triangle, Rect>> workload;
	workload.reserve(ib.GetCount() / 3);
	unsigned int blockPixelCount = 0;
	for (unsigned int j = 0; j < ib.GetCount(); j += 3)
	{
		bool discarded = false;
		for (unsigned int k = 0; k < 3; ++k) { if (vb[ib[j + k]].discard) { discarded = true; break; } }
		if (discarded) continue; // 该三角形已经放弃绘制

		Vertex* p2v[3] = { };
		vec2i min = { m_width - 1, m_height - 1 }, max = { 0, 0 };
		for (unsigned int k = 0; k < 3; ++k)
		{
			const unsigned int i = ib[j + k];
			Vertex& v = vb[i];
			p2v[k] = &vb[i];

			min.x = glm::min(min.x, (int)(v.spos.x + 0.5f)); // 整数屏幕坐标：加 0.5 的偏移取屏幕像素方格中心对齐
			min.y = glm::min(min.y, (int)(v.spos.y + 0.5f));
			max.x = glm::max(max.x, (int)(v.spos.x + 0.5f));
			max.y = glm::max(max.y, (int)(v.spos.y + 0.5f));
		}
		blockPixelCount += (max.x - min.x + 1) * (max.y - min.y + 1);
		workload.push_back({ { p2v[0], p2v[1], p2v[2] }, { min, max } });
	}

	// scan pixel block 迭代三角形外接矩形的所有点
	std::vector<std::pair<Triangle, vec3>> frags;
	frags.reserve(blockPixelCount);
	for (auto& [tri, rect] : workload)
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
		Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;

		// calculate barycentric coordinates 计算重心坐标
		vec3 p2a = a.spos - px; // 三个端点到当前点的矢量
		vec3 p2b = b.spos - px;
		vec3 p2c = c.spos - px;

		vec3 na = glm::cross(p2b, p2c);
		vec3 nb = glm::cross(p2c, p2a);
		vec3 nc = glm::cross(p2a, p2b);

		float sa = -na.z;    // 子三角形 p-b-c 面积（面朝方向为z轴正方向，叉积后的法向量的z为负）
		float sb = -nb.z;    // 子三角形 p-c-a 面积
		float sc = -nc.z;    // 子三角形 p-a-b 面积
		float s = sa + sb + sc;				   // 大三角形 a-b-c 面积

		if (sa < 0.f || sb < 0.f || sc < 0.f) { continue; } // 重心（像素）不在三角形中，背面的三角形也会被丢弃
		if (s == 0.f) { continue; } // 三角形面积为0

		float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // 得到重心坐标
		float rhw = alpha * tri.a->rhw + beta * tri.b->rhw + gamma * tri.c->rhw; // 重心插值后的w倒数
		float w = 1.f / rhw;

		// early depth test 提前深度测试

		a2v& data = a.appdata;
		a2v& ad = a.appdata, & bd = b.appdata, & cd = c.appdata;

		// barycentric interpolation 重心坐标插值
		v2f in;
		in.textures = m_textureSlots;
		for (auto& [k, v] : data.f1) { in.f1[k] = (ad.f1[k] * a.rhw * alpha + bd.f1[k] * b.rhw * beta + cd.f1[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f2) { in.f2[k] = (ad.f2[k] * a.rhw * alpha + bd.f2[k] * b.rhw * beta + cd.f2[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f3) { in.f3[k] = (ad.f3[k] * a.rhw * alpha + bd.f3[k] * b.rhw * beta + cd.f3[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f4) { in.f4[k] = (ad.f4[k] * a.rhw * alpha + bd.f4[k] * b.rhw * beta + cd.f4[k] * c.rhw * gamma) * w; }

		// fragment shader 片段着色器
		vec4 color = m_fragmentShader(in);

		// depth test 深度测试
		float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * w;
		if (z >= m_depthBuffer->data(px.x, px.y)) { continue; }
		m_depthBuffer->data(px.x, px.y) = z;

		// blending 混合
		float srcAlpha = color.a;
		vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
		vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
		m_colorBuffer->SetColor(px.x, px.y, res);
	}
}

using Range = std::pair<unsigned int, unsigned int>;

static std::vector<Range> DivideIntoRanges(unsigned int size, unsigned int rangeCount)
{
	if (size < rangeCount) return { { 0, size } };
	std::vector<Range> res;
	res.reserve(rangeCount);
	unsigned int span = size / rangeCount, even = size % rangeCount;
	unsigned int j = 0;
	for (unsigned int i = 0; i < rangeCount - 1; ++i)
	{
		unsigned int len = i < even ? span + 1 : span;
		res.push_back({ j, j + len });
		j += len;
	}
	res.push_back({ j, size });
	return res;
}

void Rasterizer::MultiThreadDraw()
{
	VertexBuffer& vb = *m_vertexBuffer;
	IndexBuffer& ib = *m_indexBuffer;

	float timeCost;
	std::vector<float> threadCost;

	std::vector<std::future<float>> futures;
	futures.reserve(m_workThreadCount);

	{
		Timer timer = { &timeCost };
		for (unsigned int i : ib)
		{
			Vertex& v = vb[i];
			v.appdata.index = i; // 设置index
		}
	}
	RST_INFO("[Index Count] {}", ib.GetCount());
	RST_INFO("[Set Index Phase] {:.2f}ms", timeCost * 1000.f);

	auto perVertex = [this, &vb](unsigned l, unsigned int r) -> float
	{
		float cost;
		{
			Timer timer = { &cost };
			for (unsigned int i = l; i < r; ++i)
			{
				Vertex& v = vb[i];
				v.discard = false; // 重置丢弃标记

				v.appdata.clear(); // 清空上下文 varying 列表

				v.pos = m_vertexShader(v.appdata); // vertex shader 运行顶点着色程序，返回顶点坐标

				// clipping 简单裁剪，任何一个顶点超过 CVV 就剔除
				if (v.pos.w == 0.0f) { v.discard = true; continue; }
				if (v.pos.z < -v.pos.w || v.pos.z > v.pos.w) { v.discard = true; continue; }
				if (v.pos.x < -v.pos.w || v.pos.x > v.pos.w) { v.discard = true; continue; }
				if (v.pos.y < -v.pos.w || v.pos.y > v.pos.w) { v.discard = true; continue; }

				// clip to ndc 透视除法
				v.rhw = 1.0f / v.pos.w;	// w 的倒数：Reciprocal of the Homogeneous W 
				v.pos /= v.pos.w;	    // 齐次坐标空间 /w 归一化到单位体积cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1

				// ndc to screen 计算屏幕坐标
				float x = (v.pos.x + 1.0f) * m_width * 0.5f;
				float y = (1.0f - v.pos.y) * m_height * 0.5f;
				v.spos = { x, y, 0.f };
			}
		}
		return cost;
	};

	{
		Timer timer = { &timeCost };
		std::vector<Range> vertexRanges = DivideIntoRanges(vb.GetCount(), m_workThreadCount);			
		threadCost.clear();
		for (Range range : vertexRanges)
		{
			unsigned int l = range.first, r = range.second;
			auto future = m_threadPool.Submit(perVertex, l, r);
			futures.push_back(std::move(future));
		}
		for (auto& future : futures) threadCost.push_back(future.get());
		futures.clear();
	}

	RST_INFO("[Per Vertex Phase] {:.2f}ms [Vertex Count] {}", timeCost * 1000.f, vb.GetCount());
	for (float cost : threadCost)
	{
		RST_TRACE("[Work Thread] {:.2f}ms", cost * 1000.f);
	}

	using Fragment = std::tuple<Triangle, vec3, vec3>;

	auto* threadFrags = new std::vector<Fragment>[m_workThreadCount];
	auto perTriangle = [this, threadFrags, &vb, &ib](unsigned l, unsigned int r, unsigned idx) -> float
	{
		float cost;
		{
			Timer time = { &cost };
			for (unsigned j = l; j < r; j += 3)
			{
				bool discarded = false;
				for (unsigned int k = 0; k < 3; ++k) if (vb[ib[j + k]].discard) { discarded = true;  break; }
				if (discarded) continue; // 该三角形已经放弃绘制

				// triangle assembly 装配三角形
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

				// scan pixel block 迭代三角形外接矩形的所有点
				Triangle tri = { p2v[0], p2v[1], p2v[2] };
				for (int y = min.y; y <= max.y; y++)
				{
					for (int x = min.x; x <= max.x; x++)
					{
						vec3 px = { (float)x + 0.5f, (float)y + 0.5f, 0.f };

						Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;

						// calculate barycentric coordinates 计算重心坐标
						vec3 p2a = a.spos - px; // 三个端点到当前点的矢量
						vec3 p2b = b.spos - px;
						vec3 p2c = c.spos - px;

						vec3 na = glm::cross(p2b, p2c);
						vec3 nb = glm::cross(p2c, p2a);
						vec3 nc = glm::cross(p2a, p2b);

						float sa = -na.z;    // 子三角形 p-b-c 面积（面朝方向为z轴正方向，叉积后的法向量的z为负）
						float sb = -nb.z;    // 子三角形 p-c-a 面积
						float sc = -nc.z;    // 子三角形 p-a-b 面积
						float s = sa + sb + sc;				   // 大三角形 a-b-c 面积

						if (sa < 0.f || sb < 0.f || sc < 0.f) { continue; } // 重心（像素）不在三角形中，背面的三角形也会被丢弃
						if (s == 0.f) { continue; } // 三角形面积为0

						float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // 得到重心坐标
						threadFrags[idx].push_back({ tri, px, { alpha, beta, gamma } });
					}
				} // potential fragment loop

			} // triangle loop
		} // timer scope
		return cost;
	};

	{
		Timer timer = { &timeCost };
		std::vector<Range> triangleRanges = DivideIntoRanges(ib.GetCount() / 3, m_workThreadCount);
		threadCost.clear();
		unsigned int idx = 0;
		for (Range range : triangleRanges)
		{
			unsigned int l = range.first * 3, r = range.second * 3;
			auto future = m_threadPool.Submit(perTriangle, l, r, idx++);
			futures.push_back(std::move(future));
		}
		for (auto& future : futures) threadCost.push_back(future.get());
		futures.clear();
	}
	RST_INFO("[Per Triangle Phase] {:.2f}ms [Triangle Count] {}", timeCost * 1000.f, ib.GetCount() / 3);
	for (float cost : threadCost)
	{
		RST_TRACE("[Work Thread] {:.2f}ms", cost * 1000.f);
	}

	std::vector<Fragment> frags;
	{
		Timer timer = { &timeCost };
		unsigned int fragCount = 0;
		for (unsigned int i = 0; i < m_workThreadCount; ++i) fragCount += threadFrags[i].size();
		frags.reserve(fragCount);
		for (unsigned int i = 0; i < m_workThreadCount; ++i)
		{
			for (auto& [tri, px, bary] : threadFrags[i]) frags.emplace_back(tri, px, bary);
		}
		delete[] threadFrags;
	}
	RST_INFO("[Fragment Merge Phase] {:.2f}ms", timeCost * 1000.f);

	// per fragment 逐片段
	auto perFragment = [this, &frags](unsigned int l, unsigned int r) -> float
	{
		float cost;
		{	
			Timer timer = { &cost };
			for (unsigned int i = l; i < r; ++i)
			{
				auto& [tri, px, bary] = frags[i];
				float alpha = bary.x, beta = bary.y, gamma = bary.z;

				Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;

				float rhw = alpha * tri.a->rhw + beta * tri.b->rhw + gamma * tri.c->rhw; // 重心插值后的w倒数
				float w = 1.f / rhw;

				a2v& data = a.appdata;
				a2v& ad = a.appdata, & bd = b.appdata, & cd = c.appdata;
				// barycentric interpolation 重心坐标插值
				v2f in;
				in.textures = m_textureSlots;
				for (auto& [k, v] : data.f1) { in.f1[k] = (ad.f1[k] * a.rhw * alpha + bd.f1[k] * b.rhw * beta + cd.f1[k] * c.rhw * gamma) * w; }
				for (auto& [k, v] : data.f2) { in.f2[k] = (ad.f2[k] * a.rhw * alpha + bd.f2[k] * b.rhw * beta + cd.f2[k] * c.rhw * gamma) * w; }
				for (auto& [k, v] : data.f3) { in.f3[k] = (ad.f3[k] * a.rhw * alpha + bd.f3[k] * b.rhw * beta + cd.f3[k] * c.rhw * gamma) * w; }
				for (auto& [k, v] : data.f4) { in.f4[k] = (ad.f4[k] * a.rhw * alpha + bd.f4[k] * b.rhw * beta + cd.f4[k] * c.rhw * gamma) * w; }

				// fragment shader 片段着色器
				vec4 color = m_fragmentShader(in);

				// depth test 深度测试
				float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * w;
				if (z >= m_depthBuffer->data(px.x, px.y)) { continue; }
				m_depthBuffer->data(px.x, px.y) = z;

				// blending 混合
				float srcAlpha = color.a;
				vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
				vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
				m_colorBuffer->SetColor(px.x, px.y, res);
			}
		}
		return cost;
	};

	{
		Timer time = { &timeCost };
		std::vector<Range> fragmentRanges = DivideIntoRanges(frags.size(), m_workThreadCount);
		threadCost.clear();
		for (Range range : fragmentRanges)
		{
			unsigned int l = range.first, r = range.second;
			auto future = m_threadPool.Submit(perFragment, l, r);
			futures.push_back(std::move(future));
		}
		for (auto& future : futures) threadCost.push_back(future.get());
		futures.clear();
	}

	RST_INFO("[Per Fragment Phase] {:.2f}ms [Fragment Count] {}", timeCost * 1000.f, frags.size());
	for (float cost : threadCost)
	{
		RST_TRACE("[Work Thread] {:.2f}ms", cost * 1000.f);
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
