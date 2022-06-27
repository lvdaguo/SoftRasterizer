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

static const unsigned int BLOCK_THREADING_AREA = 100;
static const unsigned int MAX_FRAG_COUNT = 10000000;
static float VIRTUAL_VIEWPORT_SIZE = 1.25f;

static std::unordered_map<MSAALevel, std::vector<vec3>> s_subsample_offsets_map =
{
	{
		MSAALevel::X2,
		{ 
			{ -0.25f, -0.25f, 0.0f },
			{ +0.25f, +0.25f, 0.0f }
		}
	},

	{
		MSAALevel::X4,
		{
			{ -0.25f, -0.25f, 0.0f },
			{ +0.25f, -0.25f, 0.0f },
			{ -0.25f, +0.25f, 0.0f },
			{ +0.25f, +0.25f, 0.0f }
		}
	},

	{
		MSAALevel::X8,
		{
			{ -0.125f, -0.125f, 0.0f },
			{ +0.25f, -0.25f, 0.0f },
			{ -0.25f, +0.25f, 0.0f },
			{ +0.25f, +0.25f, 0.0f },
			{ -0.25f, -0.25f, 0.0f },
			{ +0.25f, -0.25f, 0.0f },
			{ -0.25f, +0.25f, 0.0f },
			{ +0.25f, +0.25f, 0.0f }
		}
	},

	{
		MSAALevel::X16,
		{
			{ -0.125f, -0.125f, 0.0f },
			{ +0.25f, -0.25f, 0.0f },
			{ -0.25f, +0.25f, 0.0f },
			{ +0.25f, +0.25f, 0.0f },
			{ -0.25f, -0.25f, 0.0f },
			{ +0.25f, -0.25f, 0.0f },
			{ -0.25f, +0.25f, 0.0f },
			{ +0.25f, +0.25f, 0.0f },
			{ -0.25f, -0.25f, 0.0f },
			{ +0.25f, -0.25f, 0.0f },
			{ -0.25f, +0.25f, 0.0f },
			{ +0.25f, +0.25f, 0.0f },
			{ -0.25f, -0.25f, 0.0f },
			{ +0.25f, -0.25f, 0.0f },
			{ -0.25f, +0.25f, 0.0f },
			{ +0.25f, +0.25f, 0.0f }
		}
	}
};

Rasterizer::~Rasterizer() { m_threadPool.Shut(); app.RenderEvent -= m_onRender; }

void Rasterizer::Init(unsigned int workThreadCount, MSAALevel msaaLevel)
{
	m_depthTest = true;
	m_blend = true;
	m_isCullFront = false;

	m_workThreadCount = workThreadCount;
	if (workThreadCount > 0)
	{
		m_childWorkThreadCount = workThreadCount - 1; // 主线程也要完成工作量，所以要申请的子工作线程数需要减一
		m_threadPool.Init(m_childWorkThreadCount);
	}

	m_onRender = { std::bind(&Rasterizer::OnRender, this) };
	app.RenderEvent += m_onRender;

	m_width = window.GetWidth();
	m_height = window.GetHeight();

	void* bmpBuffer = CreateBitMap(window.GetHWND());

	m_clearColor = vec3(0.0f);
	
	m_colorBuffer = CreateRef<ColorBuffer>(bmpBuffer, m_width, m_height);

	m_msaaLevel = msaaLevel;
	unsigned int subsampleCount = static_cast<unsigned int>(msaaLevel);
	m_depthBuffer = CreateRef<DepthBuffer>(m_width, m_height, subsampleCount);

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
	bmpInfo.bmiHeader.biWidth = m_width;				// 指定图象的宽度，单位是象素
	bmpInfo.bmiHeader.biHeight = m_height;				// 指定图象的高度，单位是象素
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
	if (m_vertexBuffer == NULL) { TIPS(L"未绑定VertexBuffer"); return; }
	if (m_indexBuffer == NULL) { TIPS(L"未绑定IndexBuffer"); return; }
	if (m_vertexShader == NULL) { TIPS(L"未绑定VertexShader"); return; }
	if (m_fragmentShader == NULL) { TIPS(L"未绑定FragmentShader"); return; }
	
	RST_ERROR("------------------------------------------------");
	float drawCallCost;
	{
		Timer timer = { &drawCallCost };
		if (m_workThreadCount > 0)
		{
			RST_ERROR("[Draw Call] [Multi Thread] {}", m_workThreadCount);
			MultiThreadDraw();
		}
		else
		{
			RST_ERROR("[Draw Call] [Single Thread]");
			if (m_msaaLevel == MSAALevel::None) { NaiveDraw(); }
			else								{ NaiveDrawMSAA(); }
		}
	}
	RST_ERROR("[Draw Finish] cost: {:.2f}", drawCallCost * 1000.f);
	RST_ERROR("");
}

using Range = std::pair<unsigned int, unsigned int>;

// n等分一维数组
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

// n等分二维数组
static std::vector<Rect> DivideIntoRects(const Rect& rect, unsigned int rectCount)
{
	// 给工作线程划分屏幕上不同的区域
	std::vector<Rect> rects;
	rects.reserve(rectCount);

	// 只按照屏幕的高度划分，对depth buffer和color buffer的cache hit更友好
	vec2i min = { rect.min.x, rect.min.y };
	vec2i max = { rect.max.x + 1, rect.max.y + 1 };

	unsigned int height = max.y - min.y;

	std::vector<Range> heightRanges = DivideIntoRanges(height, rectCount);
	for (Range& range : heightRanges)
	{
		rects.push_back({ { min.x, min.y + range.first }, { max.x, min.y + range.second } });
	}
	return rects;
}

static bool InsideRect(const Rect& rect, const vec2i& px)
{
	return rect.min.x <= px.x && px.x < rect.max.x&& rect.min.y <= px.y && px.y < rect.max.y;
}

static unsigned int ChooseThreadBlock(const std::vector<Rect>& rects, const vec2i& px)
{
	unsigned int i = 0;
	while (i < rects.size())
	{
		if (InsideRect(rects[i], px)) break;
		++i;
	}
	return i;
}

static Rect Union(const Rect& a, const Rect& b)
{
	Rect res;
	res.min.x = glm::min(a.min.x, b.min.x);
	res.min.y = glm::min(a.min.y, b.min.y);
	res.max.x = glm::max(a.max.x, b.max.x);
	res.max.y = glm::max(a.max.y, b.max.y);
	return res;
}

void Rasterizer::MultiThreadDraw()
{
	VertexBuffer& vb = *m_vertexBuffer;
	IndexBuffer& ib = *m_indexBuffer;

	float timeCost; // 存储一个阶段的时间消耗
	std::vector<float> threadCost; // 存储每个工作线程的时间消耗

	std::vector<std::future<float>> futures;
	futures.reserve(m_childWorkThreadCount);

	// 初始化，设置索引阶段
	{
		Timer timer = { &timeCost };
		for (Vertex& v : vb) v.discard = true;

		for (unsigned int i : ib)
		{
			Vertex& v = vb[i];
			v.appdata.index = i;
			v.discard = false; // 设置到index的顶点暂时不丢弃
			v.appdata.clear(); // 清空上下文 varying 列表
		}
	}
	RST_INFO("[Index Count] {}", ib.GetCount());
	RST_INFO("[Set Index Phase] {:.2f}ms", timeCost * 1000.f);

	// 逐顶点的操作
	auto perVertex = [this, &vb](unsigned l, unsigned int r) -> float
	{
		float cost;
		{
			Timer timer = { &cost };
			for (unsigned int i = l; i < r; ++i)
			{
				Vertex& v = vb[i];
				
				if (v.discard) continue;
				v.pos = m_vertexShader(v.appdata); // vertex shader 运行顶点着色程序，返回顶点坐标

				if (v.pos.w == 0.0f) { v.discard = true; continue; }

				// clipping 简单裁剪，任何一个顶点超过 CVV 就剔除
				float bound = v.pos.w * VIRTUAL_VIEWPORT_SIZE; // 虚拟视口，比标准视口稍大，令稍微超出标准视口的小三角形依然被画出
				if (v.pos.z < -bound || v.pos.z > bound) { v.discard = true; continue; }
				if (v.pos.x < -bound || v.pos.x > bound) { v.discard = true; continue; }
				if (v.pos.y < -bound || v.pos.y > bound) { v.discard = true; continue; }

				// clip to ndc 透视除法
				v.rhw = 1.0f / v.pos.w;	// w 的倒数：Reciprocal of the Homogeneous W 
				v.pos /= v.pos.w;	    // 齐次坐标空间 /w 归一化到单位体积cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1

				// ndc to screen 计算屏幕坐标
				float x = (v.pos.x + 1.0f) * (m_width  - 1) * 0.5f;
				float y = (1.0f - v.pos.y) * (m_height - 1) * 0.5f;
				v.spos = { x, y, 0.f };
			}
		}
		return cost;
	};

	// 逐顶点阶段
	{
		Timer timer = { &timeCost };
		
		std::vector<Range> vertexRanges = DivideIntoRanges(vb.GetCount(), m_workThreadCount); // 按工作线程数划分
		threadCost.clear(); futures.clear();
		unsigned int idx = 0;
		for (Range& range : vertexRanges) 
		{
			if (idx < m_childWorkThreadCount) // 给子线程分配任务
			{
				auto future = m_threadPool.Submit(perVertex, range.first, range.second);
				futures.push_back(std::move(future));
			}
			else // 主线程完成最后一份任务
			{
				float cost = perVertex(range.first, range.second);
				threadCost.push_back(cost);
			}
			idx++;
		}
		for (auto& future : futures) threadCost.push_back(future.get()); // 完成后，等待所有子线程完成任务
	}

	RST_INFO("[Per Vertex Phase] {:.2f}ms [Vertex Count] {}", timeCost * 1000.f, vb.GetCount());
	for (float cost : threadCost)
	{
		RST_TRACE("[Work Thread] {:.2f}ms", cost * 1000.f);
	}

	// 给每个工作线程单独分配一个vector存放遍历三角形得到的像素，防止冲突
	using Fragment = std::tuple<Triangle, vec2i, vec3>;

	auto threadFrags = std::vector<std::vector<Fragment>>(m_workThreadCount);
	auto aabbs = std::vector<Rect>(m_workThreadCount, { { m_width - 1, m_height - 1 }, { 0, 0 } });

	// 逐三角形操作
	auto perTriangle = [this, &threadFrags, &aabbs, &vb, &ib](unsigned l, unsigned int r, unsigned idx) -> float
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

				aabbs[idx] = Union(aabbs[idx], { min, max });
 
				// scan pixel block 迭代三角形外接矩形的所有点
				Triangle tri = { p2v[0], p2v[1], p2v[2] };
				for (int y = min.y; y <= max.y; y++)
				{
					for (int x = min.x; x <= max.x; x++)
					{
						vec2i px = { x, y };
						vec3 pxf = { (float)px.x + 0.5f, (float)px.y + 0.5f, 0.0f };

						Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;

						// calculate barycentric coordinates 计算重心坐标
						vec3 p2a = a.spos - pxf; // 三个端点到当前点的矢量
						vec3 p2b = b.spos - pxf;
						vec3 p2c = c.spos - pxf;

						vec3 na = glm::cross(p2b, p2c);
						vec3 nb = glm::cross(p2c, p2a);
						vec3 nc = glm::cross(p2a, p2b);

						float sa = -na.z;    // 子三角形 p-b-c 面积（面朝方向为z轴正方向，叉积后的法向量的z为负）
						float sb = -nb.z;    // 子三角形 p-c-a 面积
						float sc = -nc.z;    // 子三角形 p-a-b 面积
						float s = sa + sb + sc;				   // 大三角形 a-b-c 面积

						if (!m_isCullFront && (sa < 0.f || sb < 0.f || sc < 0.f)) { continue; } // 重心（像素）不在三角形中，背面的三角形也会被丢弃
						if ( m_isCullFront && (sa > 0.f || sb > 0.f || sc > 0.f)) { continue; }
						if (s == 0.f) { continue; } // 三角形面积为0

						float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // 得到重心坐标
						
						if (0 <= x && x < (int)m_width && 0 <= y && y < (int)m_height) 
							threadFrags[idx].push_back({ tri, {x, y}, { alpha, beta, gamma } });
					}
				}

			} // triangle loop
		} // timer scope
		return cost;
	};

	// 遍历三角形阶段
	{
		Timer timer = { &timeCost };
		std::vector<Range> triangleRanges = DivideIntoRanges(ib.GetCount() / 3, m_workThreadCount);
		threadCost.clear(); futures.clear();
		unsigned int idx = 0;
		for (Range range : triangleRanges)
		{
			unsigned int l = range.first * 3, r = range.second * 3;
			if (idx < m_childWorkThreadCount) // 主线程给子线程提交任务
			{
				auto future = m_threadPool.Submit(perTriangle, l, r, idx++);
				futures.push_back(std::move(future));
			}
			else
			{
				float cost = perTriangle(l, r, idx);
				threadCost.push_back(cost); // 主线程完成的任务
			}
		}
		for (auto& future : futures) threadCost.push_back(future.get());
	}
	RST_INFO("[Per Triangle Phase] {:.2f}ms [Triangle Count] {}", timeCost * 1000.f, ib.GetCount() / 3);
	for (float cost : threadCost)
	{
		RST_TRACE("[Work Thread] {:.2f}ms", cost * 1000.f);
	}

	auto blockFrags = std::vector<std::vector<Fragment>>(m_workThreadCount);
	bool useWorkThread = true;

	// 合并之前多个线程遍历得到的片段vector
	{
		Timer timer = { &timeCost };

		Rect rect = aabbs.front();
		for (Rect& aabb : aabbs) rect = Union(rect, aabb); // 合并
		
		unsigned int rectArea = (rect.max.x - rect.min.x + 1) * (rect.max.y - rect.min.y + 1);

		if (rectArea < BLOCK_THREADING_AREA) // 面积太小，像素数量太少，直接用主线程执行逐片段操作
		{
			useWorkThread = false;
			for (auto& frags : threadFrags)
			{
				for (auto& frag : frags) blockFrags.front().push_back(frag);
			}
		}
		else
		{
			std::vector<Rect> rects =  DivideIntoRects(rect, m_workThreadCount);
			for (auto& frags : threadFrags)
			{
				for (auto& [tri, px, bary] : frags)
				{
					unsigned int i = ChooseThreadBlock(rects, px);
					blockFrags[i].push_back({ tri, px, bary });
				}
			}
		}
	}
	RST_INFO("[Fragment Merge Phase] {:.2f}ms", timeCost * 1000.f);

	// 逐片段操作
	auto perFragment = [this](const std::vector<Fragment>& frags) -> float
	{
		float cost;
		{	
			Timer timer = { &cost };
			for (auto& [tri, px, bary] : frags)
			{
				Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;
				float alpha = bary[0], beta = bary[1], gamma = bary[2];

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
				if (m_depthTest)
				{
					float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * w;
					if (z >= m_depthBuffer->GetDepth(px.x, px.y)) { continue; }
					m_depthBuffer->SetDepth(px.x, px.y, z);
				}

				if (m_blend == false) { m_colorBuffer->SetColor(px.x, px.y, color); continue; }

				// blending 混合
				float srcAlpha = color.a;
				vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
				vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
				m_colorBuffer->SetColor(px.x, px.y, res);
			}
		}
		return cost;
	};

	// 逐片段阶段
	{
		Timer time = { &timeCost };
		if (useWorkThread)
		{
			threadCost.clear(); futures.clear();
			unsigned int idx = 0;
			for (auto& frags : blockFrags)
			{
				if (idx < m_childWorkThreadCount) // 分配给子线程
				{
					auto future = m_threadPool.Submit(perFragment, frags);
					futures.push_back(std::move(future));
				}
				else // 主线程处理
				{
					float cost = perFragment(frags); 
					threadCost.push_back(cost);
				}
				idx++;
			}
			for (auto& future : futures) threadCost.push_back(future.get());
		}
		else
		{
			threadCost.clear();
			float cost = perFragment(blockFrags.front());
			threadCost.push_back(cost);
		}
	}

	unsigned int fragCount = 0;
	for (auto& frags : blockFrags) fragCount += (unsigned int)frags.size();

	RST_INFO("[Per Fragment Phase] {:.2f}ms [Fragment Count] {}", timeCost * 1000.f, fragCount);
	for (float cost : threadCost)
	{
		RST_TRACE("[Work Thread] {:.2f}ms", cost * 1000.f);
	}
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

void Rasterizer::OnRender()
{

}

void Rasterizer::NaiveDraw()
{
	VertexBuffer& vb = *m_vertexBuffer;
	IndexBuffer& ib = *m_indexBuffer;

	for (Vertex& v : vb) v.discard = true; // 重置所有顶点为丢弃状态

	// 设置index
	for (unsigned int i : ib)
	{
		Vertex& v = vb[i];
		v.appdata.index = i;
		v.discard = false; // 设置到index的顶点暂时不丢弃
		v.appdata.clear(); // 清空上下文 varying 列表
	}

	for (Vertex& v : vb)
	{
		if (v.discard) continue;

		// vertex shader 运行顶点着色程序，返回顶点坐标
		v.pos = m_vertexShader(v.appdata);

		if (v.pos.w == 0.0f) { v.discard = true; continue; }

		// clipping 简单裁剪，任何一个顶点超过 CVV 就剔除
		float bound = v.pos.w * VIRTUAL_VIEWPORT_SIZE; // 虚拟视口，比标准视口稍大，令稍微超出标准视口的小三角形依然被画出
		if (v.pos.z < -bound || v.pos.z > bound) { v.discard = true; continue; }
		if (v.pos.x < -bound || v.pos.x > bound) { v.discard = true; continue; }
		if (v.pos.y < -bound || v.pos.y > bound) { v.discard = true; continue; }

		// clip to ndc 透视除法
		v.rhw = 1.0f / v.pos.w;	// w 的倒数：Reciprocal of the Homogeneous W 
		v.pos /= v.pos.w;	    // 齐次坐标空间 /w 归一化到单位体积cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1

		// ndc to screen 计算屏幕坐标 原点在左上角
		float x = (v.pos.x + 1.0f) * (m_width  - 1) * 0.5f;
		float y = (1.0f - v.pos.y) * (m_height - 1) * 0.5f; 
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
	std::vector<std::pair<Triangle&, vec2i>> frags;
	frags.reserve(std::min(blockPixelCount, MAX_FRAG_COUNT));
	for (auto& [tri, rect] : workload)
	{
		for (int y = rect.min.y; y <= rect.max.y; y++)
		{
			for (int x = rect.min.x; x <= rect.max.x; x++)
			{
				if (0 <= x && x < (int)m_width && 0 <= y && y < (int)m_height) frags.push_back({ tri, {x, y} }); // 忽略超出屏幕范围的片段
			}
		}
	}

	// per fragment 逐片段
	for (auto& [tri, px] : frags)
	{
		Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;
		vec3 pxf = { (float)px.x + 0.5f, (float)px.y + 0.5f, 0.0f };

		// calculate barycentric coordinates 计算重心坐标
		vec3 p2a = a.spos - pxf; // 三个端点到当前点的矢量
		vec3 p2b = b.spos - pxf;
		vec3 p2c = c.spos - pxf;

		vec3 na = glm::cross(p2b, p2c);
		vec3 nb = glm::cross(p2c, p2a);
		vec3 nc = glm::cross(p2a, p2b);

		float sa = -na.z;    // 子三角形 p-b-c 面积（面朝方向为z轴正方向，叉积后的法向量的z为负）
		float sb = -nb.z;    // 子三角形 p-c-a 面积
		float sc = -nc.z;    // 子三角形 p-a-b 面积
		float s = sa + sb + sc;				   // 大三角形 a-b-c 面积

		if (!m_isCullFront && (sa < 0.f || sb < 0.f || sc < 0.f)) { continue; } // 重心（像素）不在三角形中，背面的三角形也会被丢弃
		if (m_isCullFront && (sa > 0.f || sb > 0.f || sc > 0.f)) { continue; }
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
		if (m_depthTest)
		{
			float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * w;
			if (z >= m_depthBuffer->GetDepth(px.x, px.y)) { continue; }
			m_depthBuffer->SetDepth(px.x, px.y, z);
		}

		if (m_blend == false) { m_colorBuffer->SetColor(px.x, px.y, color); continue; }

		// blending 混合
		float srcAlpha = color.a;
		vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
		vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
		m_colorBuffer->SetColor(px.x, px.y, res);
	}
}

void Rasterizer::NaiveDrawMSAA()
{
	VertexBuffer& vb = *m_vertexBuffer;
	IndexBuffer& ib = *m_indexBuffer;

	for (Vertex& v : vb) v.discard = true; // 重置所有顶点为丢弃状态

	// 设置index
	for (unsigned int i : ib)
	{
		Vertex& v = vb[i];
		v.appdata.index = i;
		v.discard = false; // 设置到index的顶点暂时不丢弃
		v.appdata.clear(); // 清空上下文 varying 列表
	}

	for (Vertex& v : vb)
	{
		if (v.discard) continue;

		// vertex shader 运行顶点着色程序，返回顶点坐标
		v.pos = m_vertexShader(v.appdata);

		if (v.pos.w == 0.0f) { v.discard = true; continue; }

		// clipping 简单裁剪，任何一个顶点超过 CVV 就剔除
		float bound = v.pos.w * VIRTUAL_VIEWPORT_SIZE; // 虚拟视口，比标准视口稍大，令稍微超出标准视口的小三角形依然被画出
		if (v.pos.z < -bound || v.pos.z > bound) { v.discard = true; continue; }
		if (v.pos.x < -bound || v.pos.x > bound) { v.discard = true; continue; }
		if (v.pos.y < -bound || v.pos.y > bound) { v.discard = true; continue; }

		// clip to ndc 透视除法
		v.rhw = 1.0f / v.pos.w;	// w 的倒数：Reciprocal of the Homogeneous W 
		v.pos /= v.pos.w;	    // 齐次坐标空间 /w 归一化到单位体积cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1

		// ndc to screen 计算屏幕坐标 原点在左上角
		float x = (v.pos.x + 1.0f) * (m_width - 1) * 0.5f;
		float y = (1.0f - v.pos.y) * (m_height - 1) * 0.5f;
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
	std::vector<std::pair<Triangle&, vec2i>> frags;
	frags.reserve(std::min(blockPixelCount, MAX_FRAG_COUNT));
	for (auto& [tri, rect] : workload)
	{
		for (int y = rect.min.y; y <= rect.max.y; y++)
		{
			for (int x = rect.min.x; x <= rect.max.x; x++)
			{
				if (0 <= x && x < (int)m_width && 0 <= y && y < (int)m_height) frags.push_back({ tri, {x, y} }); // 忽略超出屏幕范围的片段
			}
		}
	}

	// per fragment 逐片段
	for (auto& [tri, px] : frags)
	{
		Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;
		vec3 pxf = { (float)px.x + 0.5f, (float)px.y + 0.5f, 0.0f };

		auto& subsampleOffsets = s_subsample_offsets_map[m_msaaLevel];
		unsigned int coverCount = 0;
		for (unsigned int subsampleIndex = 0; subsampleIndex < subsampleOffsets.size(); subsampleIndex++)
		{
			const vec3& offset = subsampleOffsets[subsampleIndex];
			vec3 subpx = pxf + offset; // 得到子采样点

			vec3 p2a = a.spos - subpx; // 三个端点到当前子采样点的矢量
			vec3 p2b = b.spos - subpx;
			vec3 p2c = c.spos - subpx;

			vec3 na = glm::cross(p2b, p2c);
			vec3 nb = glm::cross(p2c, p2a);
			vec3 nc = glm::cross(p2a, p2b);

			float sa = -na.z;    // 子三角形 p-b-c 面积（面朝方向为z轴正方向，叉积后的法向量的z为负）
			float sb = -nb.z;    // 子三角形 p-c-a 面积
			float sc = -nc.z;    // 子三角形 p-a-b 面积
			float s = sa + sb + sc;				   // 大三角形 a-b-c 面积

			if (!m_isCullFront && (sa < 0.f || sb < 0.f || sc < 0.f)) { continue; } // 子采样点不在三角形中
			if (m_isCullFront && (sa > 0.f || sb > 0.f || sc > 0.f)) { continue; }
			if (s == 0.f) { continue; } // 三角形面积为0

			float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // 得到重心坐标
			float rhw = alpha * tri.a->rhw + beta * tri.b->rhw + gamma * tri.c->rhw; // 重心插值后的w倒数
			float w = 1.f / rhw;

			// depth test 深度测试
			if (m_depthTest)
			{
				float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * w;
				if (z >= m_depthBuffer->GetDepth(px.x, px.y, subsampleIndex)) { continue; }
				m_depthBuffer->SetDepth(px.x, px.y, z, subsampleIndex);
			}
			coverCount++;
		}

		if (coverCount == 0) { continue; } // 覆盖率为0直接丢弃

		// calculate barycentric coordinates 计算重心坐标
		vec3 p2a = a.spos - pxf; // 三个端点到当前点的矢量
		vec3 p2b = b.spos - pxf;
		vec3 p2c = c.spos - pxf;

		vec3 na = glm::cross(p2b, p2c);
		vec3 nb = glm::cross(p2c, p2a);
		vec3 nc = glm::cross(p2a, p2b);

		float sa = -na.z;    // 子三角形 p-b-c 面积（面朝方向为z轴正方向，叉积后的法向量的z为负）
		float sb = -nb.z;    // 子三角形 p-c-a 面积
		float sc = -nc.z;    // 子三角形 p-a-b 面积
		float s = sa + sb + sc;				   // 大三角形 a-b-c 面积

		float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // 得到重心坐标
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

		const unsigned int subsampleCount = static_cast<unsigned int>(m_msaaLevel);
		float coverRate = (float)coverCount / subsampleCount;

		// fragment shader 片段着色器
		vec4 color = m_fragmentShader(in);
		color.a = coverRate;

		if (m_blend == false) { m_colorBuffer->SetColor(px.x, px.y, color); continue; }

		// blending 混合
		float srcAlpha = color.a;
		vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
		vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
		m_colorBuffer->SetColor(px.x, px.y, res);

		/*
		// blending 混合
		if (coverCount == subsampleCount && m_blend == false) // 若非三角形边缘且关闭了blend，则直接设置到颜色缓冲区上
		{
			m_colorBuffer->SetColor(px.x, px.y, color);
			continue;
		}

		// 覆盖率不为1的三角形边缘片段总是被混合
		// 非三角形边缘的片段在blend开启时，正常进行混合
		float srcAlpha = coverRate;
		vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
		vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
		m_colorBuffer->SetColor(px.x, px.y, res);
		*/
	}
}

