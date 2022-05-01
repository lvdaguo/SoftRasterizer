#pragma once

#include "pch.h"
#include "Core/Buffer/VertexBuffer.h"
#include "Core/Buffer/IndexBuffer.h"
#include "Core/Graphics/Shader.h"

class VertexArray
{
public:
	VertexArray(Ref<VertexBuffer> vb, Ref<IndexBuffer> ib) : m_vb(vb), m_ib(ib) { }

public:
	Ref<VertexBuffer> GetVertexBuffer() const { return m_vb; }
	Ref<IndexBuffer> GetIndexBuffer() const { return m_ib; }

private:
	Ref<VertexBuffer> m_vb;
	Ref<IndexBuffer> m_ib;
};