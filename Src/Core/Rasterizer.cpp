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
		m_childWorkThreadCount = workThreadCount - 1; // ���߳�ҲҪ��ɹ�����������Ҫ������ӹ����߳�����Ҫ��һ
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
	// ������ͼ���õ�λͼ
	void* bmpBuffer = NULL;

	m_hDC = GetDC(hWnd);// ��ʾ
	m_hMem = CreateCompatibleDC(m_hDC);// �滭

	BITMAPINFO bmpInfo;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFO);		// ����ṹ�ĳ����ǹ̶��ģ�Ϊ40���ֽ�(LONGΪ32λ����)
	bmpInfo.bmiHeader.biWidth = m_width;				// ָ��ͼ��Ŀ�ȣ���λ������
	bmpInfo.bmiHeader.biHeight = m_height;				// ָ��ͼ��ĸ߶ȣ���λ������
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
	if (m_vertexBuffer == NULL) { TIPS(L"δ��VertexBuffer"); return; }
	if (m_indexBuffer == NULL) { TIPS(L"δ��IndexBuffer"); return; }
	if (m_vertexShader == NULL) { TIPS(L"δ��VertexShader"); return; }
	if (m_fragmentShader == NULL) { TIPS(L"δ��FragmentShader"); return; }
	
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

// n�ȷ�һά����
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

// n�ȷֶ�ά����
static std::vector<Rect> DivideIntoRects(const Rect& rect, unsigned int rectCount)
{
	// �������̻߳�����Ļ�ϲ�ͬ������
	std::vector<Rect> rects;
	rects.reserve(rectCount);

	// ֻ������Ļ�ĸ߶Ȼ��֣���depth buffer��color buffer��cache hit���Ѻ�
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

	float timeCost; // �洢һ���׶ε�ʱ������
	std::vector<float> threadCost; // �洢ÿ�������̵߳�ʱ������

	std::vector<std::future<float>> futures;
	futures.reserve(m_childWorkThreadCount);

	// ��ʼ�������������׶�
	{
		Timer timer = { &timeCost };
		for (Vertex& v : vb) v.discard = true;

		for (unsigned int i : ib)
		{
			Vertex& v = vb[i];
			v.appdata.index = i;
			v.discard = false; // ���õ�index�Ķ�����ʱ������
			v.appdata.clear(); // ��������� varying �б�
		}
	}
	RST_INFO("[Index Count] {}", ib.GetCount());
	RST_INFO("[Set Index Phase] {:.2f}ms", timeCost * 1000.f);

	// �𶥵�Ĳ���
	auto perVertex = [this, &vb](unsigned l, unsigned int r) -> float
	{
		float cost;
		{
			Timer timer = { &cost };
			for (unsigned int i = l; i < r; ++i)
			{
				Vertex& v = vb[i];
				
				if (v.discard) continue;
				v.pos = m_vertexShader(v.appdata); // vertex shader ���ж�����ɫ���򣬷��ض�������

				if (v.pos.w == 0.0f) { v.discard = true; continue; }

				// clipping �򵥲ü����κ�һ�����㳬�� CVV ���޳�
				float bound = v.pos.w * VIRTUAL_VIEWPORT_SIZE; // �����ӿڣ��ȱ�׼�ӿ��Դ�����΢������׼�ӿڵ�С��������Ȼ������
				if (v.pos.z < -bound || v.pos.z > bound) { v.discard = true; continue; }
				if (v.pos.x < -bound || v.pos.x > bound) { v.discard = true; continue; }
				if (v.pos.y < -bound || v.pos.y > bound) { v.discard = true; continue; }

				// clip to ndc ͸�ӳ���
				v.rhw = 1.0f / v.pos.w;	// w �ĵ�����Reciprocal of the Homogeneous W 
				v.pos /= v.pos.w;	    // �������ռ� /w ��һ������λ���cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1

				// ndc to screen ������Ļ����
				float x = (v.pos.x + 1.0f) * (m_width  - 1) * 0.5f;
				float y = (1.0f - v.pos.y) * (m_height - 1) * 0.5f;
				v.spos = { x, y, 0.f };
			}
		}
		return cost;
	};

	// �𶥵�׶�
	{
		Timer timer = { &timeCost };
		
		std::vector<Range> vertexRanges = DivideIntoRanges(vb.GetCount(), m_workThreadCount); // �������߳�������
		threadCost.clear(); futures.clear();
		unsigned int idx = 0;
		for (Range& range : vertexRanges) 
		{
			if (idx < m_childWorkThreadCount) // �����̷߳�������
			{
				auto future = m_threadPool.Submit(perVertex, range.first, range.second);
				futures.push_back(std::move(future));
			}
			else // ���߳�������һ������
			{
				float cost = perVertex(range.first, range.second);
				threadCost.push_back(cost);
			}
			idx++;
		}
		for (auto& future : futures) threadCost.push_back(future.get()); // ��ɺ󣬵ȴ��������߳��������
	}

	RST_INFO("[Per Vertex Phase] {:.2f}ms [Vertex Count] {}", timeCost * 1000.f, vb.GetCount());
	for (float cost : threadCost)
	{
		RST_TRACE("[Work Thread] {:.2f}ms", cost * 1000.f);
	}

	// ��ÿ�������̵߳�������һ��vector��ű��������εõ������أ���ֹ��ͻ
	using Fragment = std::tuple<Triangle, vec2i, vec3>;

	auto threadFrags = std::vector<std::vector<Fragment>>(m_workThreadCount);
	auto aabbs = std::vector<Rect>(m_workThreadCount, { { m_width - 1, m_height - 1 }, { 0, 0 } });

	// �������β���
	auto perTriangle = [this, &threadFrags, &aabbs, &vb, &ib](unsigned l, unsigned int r, unsigned idx) -> float
	{
		float cost;
		{
			Timer time = { &cost };
			for (unsigned j = l; j < r; j += 3)
			{
				bool discarded = false;
				for (unsigned int k = 0; k < 3; ++k) if (vb[ib[j + k]].discard) { discarded = true;  break; }
				if (discarded) continue; // ���������Ѿ���������

				// triangle assembly װ��������
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

				aabbs[idx] = Union(aabbs[idx], { min, max });
 
				// scan pixel block ������������Ӿ��ε����е�
				Triangle tri = { p2v[0], p2v[1], p2v[2] };
				for (int y = min.y; y <= max.y; y++)
				{
					for (int x = min.x; x <= max.x; x++)
					{
						vec2i px = { x, y };
						vec3 pxf = { (float)px.x + 0.5f, (float)px.y + 0.5f, 0.0f };

						Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;

						// calculate barycentric coordinates ������������
						vec3 p2a = a.spos - pxf; // �����˵㵽��ǰ���ʸ��
						vec3 p2b = b.spos - pxf;
						vec3 p2c = c.spos - pxf;

						vec3 na = glm::cross(p2b, p2c);
						vec3 nb = glm::cross(p2c, p2a);
						vec3 nc = glm::cross(p2a, p2b);

						float sa = -na.z;    // �������� p-b-c ������泯����Ϊz�������򣬲����ķ�������zΪ����
						float sb = -nb.z;    // �������� p-c-a ���
						float sc = -nc.z;    // �������� p-a-b ���
						float s = sa + sb + sc;				   // �������� a-b-c ���

						if (!m_isCullFront && (sa < 0.f || sb < 0.f || sc < 0.f)) { continue; } // ���ģ����أ������������У������������Ҳ�ᱻ����
						if ( m_isCullFront && (sa > 0.f || sb > 0.f || sc > 0.f)) { continue; }
						if (s == 0.f) { continue; } // ���������Ϊ0

						float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // �õ���������
						
						if (0 <= x && x < (int)m_width && 0 <= y && y < (int)m_height) 
							threadFrags[idx].push_back({ tri, {x, y}, { alpha, beta, gamma } });
					}
				}

			} // triangle loop
		} // timer scope
		return cost;
	};

	// ���������ν׶�
	{
		Timer timer = { &timeCost };
		std::vector<Range> triangleRanges = DivideIntoRanges(ib.GetCount() / 3, m_workThreadCount);
		threadCost.clear(); futures.clear();
		unsigned int idx = 0;
		for (Range range : triangleRanges)
		{
			unsigned int l = range.first * 3, r = range.second * 3;
			if (idx < m_childWorkThreadCount) // ���̸߳����߳��ύ����
			{
				auto future = m_threadPool.Submit(perTriangle, l, r, idx++);
				futures.push_back(std::move(future));
			}
			else
			{
				float cost = perTriangle(l, r, idx);
				threadCost.push_back(cost); // ���߳���ɵ�����
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

	// �ϲ�֮ǰ����̱߳����õ���Ƭ��vector
	{
		Timer timer = { &timeCost };

		Rect rect = aabbs.front();
		for (Rect& aabb : aabbs) rect = Union(rect, aabb); // �ϲ�
		
		unsigned int rectArea = (rect.max.x - rect.min.x + 1) * (rect.max.y - rect.min.y + 1);

		if (rectArea < BLOCK_THREADING_AREA) // ���̫С����������̫�٣�ֱ�������߳�ִ����Ƭ�β���
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

	// ��Ƭ�β���
	auto perFragment = [this](const std::vector<Fragment>& frags) -> float
	{
		float cost;
		{	
			Timer timer = { &cost };
			for (auto& [tri, px, bary] : frags)
			{
				Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;
				float alpha = bary[0], beta = bary[1], gamma = bary[2];

				float rhw = alpha * tri.a->rhw + beta * tri.b->rhw + gamma * tri.c->rhw; // ���Ĳ�ֵ���w����
				float w = 1.f / rhw;

				a2v& data = a.appdata;
				a2v& ad = a.appdata, & bd = b.appdata, & cd = c.appdata;
				// barycentric interpolation ���������ֵ
				v2f in;
				in.textures = m_textureSlots;
				for (auto& [k, v] : data.f1) { in.f1[k] = (ad.f1[k] * a.rhw * alpha + bd.f1[k] * b.rhw * beta + cd.f1[k] * c.rhw * gamma) * w; }
				for (auto& [k, v] : data.f2) { in.f2[k] = (ad.f2[k] * a.rhw * alpha + bd.f2[k] * b.rhw * beta + cd.f2[k] * c.rhw * gamma) * w; }
				for (auto& [k, v] : data.f3) { in.f3[k] = (ad.f3[k] * a.rhw * alpha + bd.f3[k] * b.rhw * beta + cd.f3[k] * c.rhw * gamma) * w; }
				for (auto& [k, v] : data.f4) { in.f4[k] = (ad.f4[k] * a.rhw * alpha + bd.f4[k] * b.rhw * beta + cd.f4[k] * c.rhw * gamma) * w; }

				// fragment shader Ƭ����ɫ��
				vec4 color = m_fragmentShader(in);

				// depth test ��Ȳ���
				if (m_depthTest)
				{
					float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * w;
					if (z >= m_depthBuffer->GetDepth(px.x, px.y)) { continue; }
					m_depthBuffer->SetDepth(px.x, px.y, z);
				}

				if (m_blend == false) { m_colorBuffer->SetColor(px.x, px.y, color); continue; }

				// blending ���
				float srcAlpha = color.a;
				vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
				vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
				m_colorBuffer->SetColor(px.x, px.y, res);
			}
		}
		return cost;
	};

	// ��Ƭ�ν׶�
	{
		Timer time = { &timeCost };
		if (useWorkThread)
		{
			threadCost.clear(); futures.clear();
			unsigned int idx = 0;
			for (auto& frags : blockFrags)
			{
				if (idx < m_childWorkThreadCount) // ��������߳�
				{
					auto future = m_threadPool.Submit(perFragment, frags);
					futures.push_back(std::move(future));
				}
				else // ���̴߳���
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
	// �����ﻭ���豸�ϣ�hMem�൱�ڻ�����
	BitBlt(m_hDC, 0, 0, m_width, m_height, m_hMem, 0, 0, SRCCOPY);
	// �ú�����ָ����Դ�豸���������е����ؽ���λ�飨bit_block��ת�����Դ��͵�Ŀ���豸����
}

void Rasterizer::OnRender()
{

}

void Rasterizer::NaiveDraw()
{
	VertexBuffer& vb = *m_vertexBuffer;
	IndexBuffer& ib = *m_indexBuffer;

	for (Vertex& v : vb) v.discard = true; // �������ж���Ϊ����״̬

	// ����index
	for (unsigned int i : ib)
	{
		Vertex& v = vb[i];
		v.appdata.index = i;
		v.discard = false; // ���õ�index�Ķ�����ʱ������
		v.appdata.clear(); // ��������� varying �б�
	}

	for (Vertex& v : vb)
	{
		if (v.discard) continue;

		// vertex shader ���ж�����ɫ���򣬷��ض�������
		v.pos = m_vertexShader(v.appdata);

		if (v.pos.w == 0.0f) { v.discard = true; continue; }

		// clipping �򵥲ü����κ�һ�����㳬�� CVV ���޳�
		float bound = v.pos.w * VIRTUAL_VIEWPORT_SIZE; // �����ӿڣ��ȱ�׼�ӿ��Դ�����΢������׼�ӿڵ�С��������Ȼ������
		if (v.pos.z < -bound || v.pos.z > bound) { v.discard = true; continue; }
		if (v.pos.x < -bound || v.pos.x > bound) { v.discard = true; continue; }
		if (v.pos.y < -bound || v.pos.y > bound) { v.discard = true; continue; }

		// clip to ndc ͸�ӳ���
		v.rhw = 1.0f / v.pos.w;	// w �ĵ�����Reciprocal of the Homogeneous W 
		v.pos /= v.pos.w;	    // �������ռ� /w ��һ������λ���cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1

		// ndc to screen ������Ļ���� ԭ�������Ͻ�
		float x = (v.pos.x + 1.0f) * (m_width  - 1) * 0.5f;
		float y = (1.0f - v.pos.y) * (m_height - 1) * 0.5f; 
		v.spos = { x, y, 0.f };
	}

	// triangle assembly װ��������
	std::vector<std::pair<Triangle, Rect>> workload;
	workload.reserve(ib.GetCount() / 3);
	unsigned int blockPixelCount = 0;
	for (unsigned int j = 0; j < ib.GetCount(); j += 3)
	{
		bool discarded = false;
		for (unsigned int k = 0; k < 3; ++k) { if (vb[ib[j + k]].discard) { discarded = true; break; } }
		if (discarded) continue; // ���������Ѿ���������

		Vertex* p2v[3] = { };
		vec2i min = { m_width - 1, m_height - 1 }, max = { 0, 0 };
		for (unsigned int k = 0; k < 3; ++k)
		{
			const unsigned int i = ib[j + k];
			Vertex& v = vb[i];
			p2v[k] = &vb[i];

			min.x = glm::min(min.x, (int)(v.spos.x + 0.5f)); // ������Ļ���꣺�� 0.5 ��ƫ��ȡ��Ļ���ط������Ķ���
			min.y = glm::min(min.y, (int)(v.spos.y + 0.5f));
			max.x = glm::max(max.x, (int)(v.spos.x + 0.5f));
			max.y = glm::max(max.y, (int)(v.spos.y + 0.5f));
		}
		blockPixelCount += (max.x - min.x + 1) * (max.y - min.y + 1);
		workload.push_back({ { p2v[0], p2v[1], p2v[2] }, { min, max } });
	}

	// scan pixel block ������������Ӿ��ε����е�
	std::vector<std::pair<Triangle&, vec2i>> frags;
	frags.reserve(std::min(blockPixelCount, MAX_FRAG_COUNT));
	for (auto& [tri, rect] : workload)
	{
		for (int y = rect.min.y; y <= rect.max.y; y++)
		{
			for (int x = rect.min.x; x <= rect.max.x; x++)
			{
				if (0 <= x && x < (int)m_width && 0 <= y && y < (int)m_height) frags.push_back({ tri, {x, y} }); // ���Գ�����Ļ��Χ��Ƭ��
			}
		}
	}

	// per fragment ��Ƭ��
	for (auto& [tri, px] : frags)
	{
		Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;
		vec3 pxf = { (float)px.x + 0.5f, (float)px.y + 0.5f, 0.0f };

		// calculate barycentric coordinates ������������
		vec3 p2a = a.spos - pxf; // �����˵㵽��ǰ���ʸ��
		vec3 p2b = b.spos - pxf;
		vec3 p2c = c.spos - pxf;

		vec3 na = glm::cross(p2b, p2c);
		vec3 nb = glm::cross(p2c, p2a);
		vec3 nc = glm::cross(p2a, p2b);

		float sa = -na.z;    // �������� p-b-c ������泯����Ϊz�������򣬲����ķ�������zΪ����
		float sb = -nb.z;    // �������� p-c-a ���
		float sc = -nc.z;    // �������� p-a-b ���
		float s = sa + sb + sc;				   // �������� a-b-c ���

		if (!m_isCullFront && (sa < 0.f || sb < 0.f || sc < 0.f)) { continue; } // ���ģ����أ������������У������������Ҳ�ᱻ����
		if (m_isCullFront && (sa > 0.f || sb > 0.f || sc > 0.f)) { continue; }
		if (s == 0.f) { continue; } // ���������Ϊ0

		float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // �õ���������
		float rhw = alpha * tri.a->rhw + beta * tri.b->rhw + gamma * tri.c->rhw; // ���Ĳ�ֵ���w����
		float w = 1.f / rhw;

		// early depth test ��ǰ��Ȳ���

		a2v& data = a.appdata;
		a2v& ad = a.appdata, & bd = b.appdata, & cd = c.appdata;

		// barycentric interpolation ���������ֵ
		v2f in;
		in.textures = m_textureSlots;
		for (auto& [k, v] : data.f1) { in.f1[k] = (ad.f1[k] * a.rhw * alpha + bd.f1[k] * b.rhw * beta + cd.f1[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f2) { in.f2[k] = (ad.f2[k] * a.rhw * alpha + bd.f2[k] * b.rhw * beta + cd.f2[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f3) { in.f3[k] = (ad.f3[k] * a.rhw * alpha + bd.f3[k] * b.rhw * beta + cd.f3[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f4) { in.f4[k] = (ad.f4[k] * a.rhw * alpha + bd.f4[k] * b.rhw * beta + cd.f4[k] * c.rhw * gamma) * w; }

		// fragment shader Ƭ����ɫ��
		vec4 color = m_fragmentShader(in);

		// depth test ��Ȳ���
		if (m_depthTest)
		{
			float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * w;
			if (z >= m_depthBuffer->GetDepth(px.x, px.y)) { continue; }
			m_depthBuffer->SetDepth(px.x, px.y, z);
		}

		if (m_blend == false) { m_colorBuffer->SetColor(px.x, px.y, color); continue; }

		// blending ���
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

	for (Vertex& v : vb) v.discard = true; // �������ж���Ϊ����״̬

	// ����index
	for (unsigned int i : ib)
	{
		Vertex& v = vb[i];
		v.appdata.index = i;
		v.discard = false; // ���õ�index�Ķ�����ʱ������
		v.appdata.clear(); // ��������� varying �б�
	}

	for (Vertex& v : vb)
	{
		if (v.discard) continue;

		// vertex shader ���ж�����ɫ���򣬷��ض�������
		v.pos = m_vertexShader(v.appdata);

		if (v.pos.w == 0.0f) { v.discard = true; continue; }

		// clipping �򵥲ü����κ�һ�����㳬�� CVV ���޳�
		float bound = v.pos.w * VIRTUAL_VIEWPORT_SIZE; // �����ӿڣ��ȱ�׼�ӿ��Դ�����΢������׼�ӿڵ�С��������Ȼ������
		if (v.pos.z < -bound || v.pos.z > bound) { v.discard = true; continue; }
		if (v.pos.x < -bound || v.pos.x > bound) { v.discard = true; continue; }
		if (v.pos.y < -bound || v.pos.y > bound) { v.discard = true; continue; }

		// clip to ndc ͸�ӳ���
		v.rhw = 1.0f / v.pos.w;	// w �ĵ�����Reciprocal of the Homogeneous W 
		v.pos /= v.pos.w;	    // �������ռ� /w ��һ������λ���cvv: x - [-1, 1], y - [-1, 1], z - [-1, 1], w - 1

		// ndc to screen ������Ļ���� ԭ�������Ͻ�
		float x = (v.pos.x + 1.0f) * (m_width - 1) * 0.5f;
		float y = (1.0f - v.pos.y) * (m_height - 1) * 0.5f;
		v.spos = { x, y, 0.f };
	}

	// triangle assembly װ��������
	std::vector<std::pair<Triangle, Rect>> workload;
	workload.reserve(ib.GetCount() / 3);
	unsigned int blockPixelCount = 0;
	for (unsigned int j = 0; j < ib.GetCount(); j += 3)
	{
		bool discarded = false;
		for (unsigned int k = 0; k < 3; ++k) { if (vb[ib[j + k]].discard) { discarded = true; break; } }
		if (discarded) continue; // ���������Ѿ���������

		Vertex* p2v[3] = { };
		vec2i min = { m_width - 1, m_height - 1 }, max = { 0, 0 };
		for (unsigned int k = 0; k < 3; ++k)
		{
			const unsigned int i = ib[j + k];
			Vertex& v = vb[i];
			p2v[k] = &vb[i];

			min.x = glm::min(min.x, (int)(v.spos.x + 0.5f)); // ������Ļ���꣺�� 0.5 ��ƫ��ȡ��Ļ���ط������Ķ���
			min.y = glm::min(min.y, (int)(v.spos.y + 0.5f));
			max.x = glm::max(max.x, (int)(v.spos.x + 0.5f));
			max.y = glm::max(max.y, (int)(v.spos.y + 0.5f));
		}
		blockPixelCount += (max.x - min.x + 1) * (max.y - min.y + 1);
		workload.push_back({ { p2v[0], p2v[1], p2v[2] }, { min, max } });
	}

	// scan pixel block ������������Ӿ��ε����е�
	std::vector<std::pair<Triangle&, vec2i>> frags;
	frags.reserve(std::min(blockPixelCount, MAX_FRAG_COUNT));
	for (auto& [tri, rect] : workload)
	{
		for (int y = rect.min.y; y <= rect.max.y; y++)
		{
			for (int x = rect.min.x; x <= rect.max.x; x++)
			{
				if (0 <= x && x < (int)m_width && 0 <= y && y < (int)m_height) frags.push_back({ tri, {x, y} }); // ���Գ�����Ļ��Χ��Ƭ��
			}
		}
	}

	// per fragment ��Ƭ��
	for (auto& [tri, px] : frags)
	{
		Vertex& a = *tri.a, & b = *tri.b, & c = *tri.c;
		vec3 pxf = { (float)px.x + 0.5f, (float)px.y + 0.5f, 0.0f };

		auto& subsampleOffsets = s_subsample_offsets_map[m_msaaLevel];
		unsigned int coverCount = 0;
		for (unsigned int subsampleIndex = 0; subsampleIndex < subsampleOffsets.size(); subsampleIndex++)
		{
			const vec3& offset = subsampleOffsets[subsampleIndex];
			vec3 subpx = pxf + offset; // �õ��Ӳ�����

			vec3 p2a = a.spos - subpx; // �����˵㵽��ǰ�Ӳ������ʸ��
			vec3 p2b = b.spos - subpx;
			vec3 p2c = c.spos - subpx;

			vec3 na = glm::cross(p2b, p2c);
			vec3 nb = glm::cross(p2c, p2a);
			vec3 nc = glm::cross(p2a, p2b);

			float sa = -na.z;    // �������� p-b-c ������泯����Ϊz�������򣬲����ķ�������zΪ����
			float sb = -nb.z;    // �������� p-c-a ���
			float sc = -nc.z;    // �������� p-a-b ���
			float s = sa + sb + sc;				   // �������� a-b-c ���

			if (!m_isCullFront && (sa < 0.f || sb < 0.f || sc < 0.f)) { continue; } // �Ӳ����㲻����������
			if (m_isCullFront && (sa > 0.f || sb > 0.f || sc > 0.f)) { continue; }
			if (s == 0.f) { continue; } // ���������Ϊ0

			float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // �õ���������
			float rhw = alpha * tri.a->rhw + beta * tri.b->rhw + gamma * tri.c->rhw; // ���Ĳ�ֵ���w����
			float w = 1.f / rhw;

			// depth test ��Ȳ���
			if (m_depthTest)
			{
				float z = (a.pos.z * alpha + b.pos.z * beta + c.pos.z * gamma) * w;
				if (z >= m_depthBuffer->GetDepth(px.x, px.y, subsampleIndex)) { continue; }
				m_depthBuffer->SetDepth(px.x, px.y, z, subsampleIndex);
			}
			coverCount++;
		}

		if (coverCount == 0) { continue; } // ������Ϊ0ֱ�Ӷ���

		// calculate barycentric coordinates ������������
		vec3 p2a = a.spos - pxf; // �����˵㵽��ǰ���ʸ��
		vec3 p2b = b.spos - pxf;
		vec3 p2c = c.spos - pxf;

		vec3 na = glm::cross(p2b, p2c);
		vec3 nb = glm::cross(p2c, p2a);
		vec3 nc = glm::cross(p2a, p2b);

		float sa = -na.z;    // �������� p-b-c ������泯����Ϊz�������򣬲����ķ�������zΪ����
		float sb = -nb.z;    // �������� p-c-a ���
		float sc = -nc.z;    // �������� p-a-b ���
		float s = sa + sb + sc;				   // �������� a-b-c ���

		float alpha = sa / s, beta = sb / s, gamma = sc / s;					 // �õ���������
		float rhw = alpha * tri.a->rhw + beta * tri.b->rhw + gamma * tri.c->rhw; // ���Ĳ�ֵ���w����
		float w = 1.f / rhw;

		a2v& data = a.appdata;
		a2v& ad = a.appdata, & bd = b.appdata, & cd = c.appdata;

		// barycentric interpolation ���������ֵ
		v2f in;
		in.textures = m_textureSlots;
		for (auto& [k, v] : data.f1) { in.f1[k] = (ad.f1[k] * a.rhw * alpha + bd.f1[k] * b.rhw * beta + cd.f1[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f2) { in.f2[k] = (ad.f2[k] * a.rhw * alpha + bd.f2[k] * b.rhw * beta + cd.f2[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f3) { in.f3[k] = (ad.f3[k] * a.rhw * alpha + bd.f3[k] * b.rhw * beta + cd.f3[k] * c.rhw * gamma) * w; }
		for (auto& [k, v] : data.f4) { in.f4[k] = (ad.f4[k] * a.rhw * alpha + bd.f4[k] * b.rhw * beta + cd.f4[k] * c.rhw * gamma) * w; }

		const unsigned int subsampleCount = static_cast<unsigned int>(m_msaaLevel);
		float coverRate = (float)coverCount / subsampleCount;

		// fragment shader Ƭ����ɫ��
		vec4 color = m_fragmentShader(in);
		color.a = coverRate;

		if (m_blend == false) { m_colorBuffer->SetColor(px.x, px.y, color); continue; }

		// blending ���
		float srcAlpha = color.a;
		vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
		vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
		m_colorBuffer->SetColor(px.x, px.y, res);

		/*
		// blending ���
		if (coverCount == subsampleCount && m_blend == false) // ���������α�Ե�ҹر���blend����ֱ�����õ���ɫ��������
		{
			m_colorBuffer->SetColor(px.x, px.y, color);
			continue;
		}

		// �����ʲ�Ϊ1�������α�ԵƬ�����Ǳ����
		// �������α�Ե��Ƭ����blend����ʱ���������л��
		float srcAlpha = coverRate;
		vec3 col1 = color, col2 = m_colorBuffer->GetColor(px.x, px.y);
		vec3 res = srcAlpha * col1 + (1.f - srcAlpha) * col2;
		m_colorBuffer->SetColor(px.x, px.y, res);
		*/
	}
}

