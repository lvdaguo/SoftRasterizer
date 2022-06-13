#include "pch.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "Model.h"

Model::Model(const std::string& path)
{
    LoadMesh(path);
}

void Model::LoadMesh(const std::string& path)
{
    Assimp::Importer importer;

    // aiProcess_Triangulate        将四边形，五边形等多边形都转化为三角形
    // aiProcess_FixInfacingNormals 程序修复正面和反面的错误
    //| aiProcess_FixInfacingNormals
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
    {
        LOG_ERROR("加载模型错误，路径为{}", path);
        return;
    }
    m_directory = path.substr(0, path.find_last_of('/'));
    TraverseNode(scene->mRootNode, scene);
}

void Model::TraverseNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(GenerateMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        TraverseNode(node->mChildren[i], scene);
    }
}

Ref<Mesh> Model::GenerateMesh(aiMesh* mesh, const aiScene* scene)
{
    Ref<std::vector<VertexData>> vertexes = GenerateVertexes(mesh);
    Ref<std::vector<unsigned int>> indices = GenerateIndices(mesh);
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Ref<Texture>> diffuseTextures = LoadMaterialTextures(material, aiTextureType_DIFFUSE);
    std::vector<Ref<Texture>> specularTextures = LoadMaterialTextures(material, aiTextureType_SPECULAR);
    return CreateRef<Mesh>(vertexes, indices, diffuseTextures, specularTextures);
}

Ref<std::vector<VertexData>> Model::GenerateVertexes(aiMesh* mesh) const
{
    auto vertices = CreateRef<std::vector<VertexData>>();
    vertices->reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        VertexData vertex;
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normal.x = mesh->mNormals[i].x;
        vertex.normal.y = mesh->mNormals[i].y;
        vertex.normal.z = mesh->mNormals[i].z;

        // 网格是否有纹理坐标
        if (mesh->mTextureCoords[0])
        {
            vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertex.texCoords = glm::vec2(0.0f);
        }

        vertices->push_back(vertex);
    }
    return vertices;
}

Ref<std::vector<unsigned int>> Model::GenerateIndices(aiMesh* mesh) const
{
    auto indices = CreateRef<std::vector<unsigned int>>();
    indices->reserve(mesh->mNumFaces * 3);                   // 这里写死了每个面必须是三角形
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            indices->push_back(face.mIndices[j]);
        }
    }
    return indices;
}

std::vector<Ref<Texture>> Model::LoadMaterialTextures(aiMaterial* material, aiTextureType type) const
{
    std::vector<Ref<Texture>> textures;
    static std::unordered_map<std::string, Ref<Texture>> loadedTextures;

    for (unsigned int i = 0; i < material->GetTextureCount(type); ++i)
    {
        aiString str;
        material->GetTexture(type, i, &str);
        std::string filename(str.C_Str());

        // 共享已经加载过的纹理
        auto it = loadedTextures.find(filename);
        if (it == loadedTextures.end())
        {
            std::string filepath = m_directory + '/' + filename;
            textures.push_back(CreateRef<Texture>(filepath));
            loadedTextures.emplace(filename, textures.back());
        }
        else
        {
            textures.push_back(loadedTextures[filename]);
        }
    }

    return textures;
}

