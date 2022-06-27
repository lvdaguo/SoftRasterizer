#pragma once

#include <assimp/scene.h>
#include "Core/Buffer/VertexBuffer.h"
#include "Core/Buffer/IndexBuffer.h"
#include "Core/Buffer/VertexArray.h"

/// <summary>
/// 顶点（纯数据结构体）
/// </summary>
struct VertexData
{
	vec3 position;
	vec3 normal;
	vec2 texCoords;
};

/// <summary>
/// 网格
/// </summary>
class Mesh
{
public:
	Mesh(Ref<std::vector<VertexData>> vertices,
		Ref<std::vector<unsigned int>> indices,
		const std::vector<Ref<Texture>>& diffuseTextures,
		const std::vector<Ref<Texture>>& specularTextures)
		: m_verticesRefHolder(vertices), m_indicesRefHolder(indices)
	{
		m_diffuseTextures = diffuseTextures;
		m_specularTextures = specularTextures;

		m_vb = CreateRef<VertexBuffer>((void*)vertices->data(), (unsigned int)vertices->size());
		m_ib = CreateRef<IndexBuffer>((unsigned int*)indices->data(), (unsigned int)indices->size());
		m_va = CreateRef<VertexArray>(m_vb, m_ib);
	}

public:
	Ref<VertexBuffer> GetVertexBuffer() const { return m_vb; }
	Ref<IndexBuffer> GetIndexBuffer() const { return m_ib; }
	Ref<VertexArray> GetVertexArray() const { return m_va; }
	std::vector<Ref<Texture>> GetDiffuseTextures() const { return m_diffuseTextures; }
	std::vector<Ref<Texture>> GetSpecularTextures() const { return m_specularTextures; }

private:
	Ref<std::vector<VertexData>> m_verticesRefHolder; // 维持引用计数，防止被释放，导致访问错误
	Ref<std::vector<unsigned int>> m_indicesRefHolder;

	std::vector<Ref<Texture>> m_diffuseTextures;
	std::vector<Ref<Texture>> m_specularTextures;
	Ref<VertexBuffer> m_vb;
	Ref<IndexBuffer> m_ib;
	Ref<VertexArray> m_va;
};

/// <summary>
/// 模型（可由多个网格组成）
/// </summary>
class Model
{
public:
	Model(const std::string& path);

public:
	std::vector<Ref<Mesh>> GetMeshes() const { return m_meshes; }
	auto begin() { return m_meshes.begin(); }
	auto end() { return m_meshes.end(); }
	auto begin() const { return m_meshes.cbegin(); }
	auto end() const { return m_meshes.cend(); }

private:
	void LoadMesh(const std::string& path);

	std::vector<Ref<Mesh>> m_meshes;
	std::string m_directory;

	void TraverseNode(aiNode* node, const aiScene* scene);
	Ref<Mesh> GenerateMesh(aiMesh* mesh, const aiScene* scene);

	Ref<std::vector<VertexData>> GenerateVertexes(aiMesh* mesh) const;
	Ref<std::vector<unsigned int>> GenerateIndices(aiMesh* mesh) const;
	std::vector<Ref<Texture>> LoadMaterialTextures(aiMaterial* matrial, aiTextureType type) const;
};


